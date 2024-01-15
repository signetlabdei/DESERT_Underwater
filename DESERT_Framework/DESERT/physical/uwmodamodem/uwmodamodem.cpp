//
// Copyright (c) 2019 Regents of the SIGNET lab, University of Padova.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the University of Padova (SIGNET lab) nor the
//    names of its contributors may be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <uwmodamodem.h>
#include <uwsocket.h>

#include <string>
#include <functional>
#include <algorithm>

#include <iostream>

const int UwMODAModem::SIGNALING_ADDRESS = 55006;

const int UwMODAModem::DATA_ADDRESS = 55555;

const std::string sep = {"::"};

const std::string end_delim = {";"};

const std::chrono::milliseconds UwMODAModem::MODEM_TIMEOUT =
    std::chrono::milliseconds(3000);

std::vector<std::pair<std::string, UwMODAModem::ModemResponse> >
  UwMODAModem::signaling_dict {
    std::make_pair("RX_STARTED", ModemResponse::RX_BEG),
    std::make_pair("TX_ENDED",   ModemResponse::TX_END),
    std::make_pair("CFG_ENDED",  ModemResponse::CFG_END)
};

std::map<UwMODAModem::ModemState, std::string>
  UwMODAModem::stateToString {
    std::make_pair(ModemState::AVAILABLE, "AVAILABLE"),
    std::make_pair(ModemState::TRANSMITTING, "TRANSMITTING"),
    std::make_pair(ModemState::RECEIVING, "RECEIVING"),
    std::make_pair(ModemState::CONFIGURING, "CONFIGURING")
};

/**
 * Class to create the Otcl shadow object for an object of the class
 * UwMODAModem.
 */
static class UwMODAModem_TclClass : public TclClass
{

public:
  UwMODAModem_TclClass()
    : TclClass("Module/UW/UwModem/MODA")
  {
  }

  TclObject *
  create(int args, const char *const *argv)
  {
    return (new UwMODAModem());
  }

} class_modamodem;

UwMODAModem::UwMODAModem()
  : UwModem()
  , status(ModemState::AVAILABLE)
  , status_m()
  , tx_queue_m()
  , tx_queue_cv()
  , receiving(false)
  , transmitting(false)
  , rx_payload("")
  , sig_thread()
  , rx_thread()
  , tx_thread()
  , signal_conn(new UwSocket())
  , data_conn(new UwSocket())
  , signal_buffer()
  , signal_tag("DRIVER")
  , signal_address("")
{
  signal_conn->setTCP();
  data_conn->setTCP();
  data_buffer.clear();
}

UwMODAModem::~UwMODAModem()
{
    stop();
}

void
UwMODAModem::recv(Packet* p)
{
    hdr_cmn *ch = HDR_CMN(p);
    hdr_MPhy *ph = HDR_MPHY(p);

    if (ch->direction() == hdr_cmn::UP) {
      if (isOn) {
        startRx(p);
        endRx(p);
      } else {
        Packet::free(p);
      }
    } else { // hdr_cmn::DOWN
        if (!isOn) {
          return;
        }
        ph->Pr = 0;
        ph->Pn = 0;
        ph->Pi = 0;
        ph->txtime = NOW;
        ph->rxtime = ph->txtime;

        ph->worth_tracing = false;

        ph->srcSpectralMask = getTxSpectralMask(p);
        ph->srcAntenna = getTxAntenna(p);
        ph->srcPosition = getPosition();
        ph->dstSpectralMask = 0;
        ph->dstPosition = 0;
        ph->dstAntenna = 0;
        ph->modulationType = getModulationType(p);
        ph->duration = getTxDuration(p);

        std::unique_lock<std::mutex> tx_lock(tx_queue_m);
        tx_queue.push(p);
        tx_lock.unlock();
        printOnLog(LogLevel::DEBUG, "MODAMODEM", "recv::PUSHING_IN_TX_QUEUE");
        tx_queue_cv.notify_one();
    }
    return;
}

int
UwMODAModem::command(int argc, const char *const *argv)
{
  // Tcl &tcl = Tcl::instance();

  if (argc == 3) {
    if (!strcmp(argv[1], "setSignalAddress")) {
      signal_address = argv[2];
      return TCL_OK;
    }
  }
  return UwModem::command(argc, argv);
}

int
UwMODAModem::getModulationType(Packet* P)
{
    return 0;
}

int
UwMODAModem::recvSyncClMsg(ClMessage *m)
{
  return 0;
}

void
UwMODAModem::startRx(Packet *p)
{
  printOnLog(LogLevel::INFO, "MODAMODEM",
      "startRx::CALL_PHY2MACSTARTRX");
  Phy2MacStartRx(p);
}

void
UwMODAModem::endRx(Packet *p)
{
  printOnLog(LogLevel::INFO, "MODAMODEM", "endRx::CALL_SENDUP");
  sendUp(p, 0.01);
}

void
UwMODAModem::start()
{
  printOnLog(LogLevel::DEBUG, "MODAMODEM", "STARTING_DRIVER");

  if (!signal_conn->openConnection(signal_address)) {
    std::string err_msg = "SIGNALING_CHANNEL_FAILED_TO_OPEN_AT_PORT:"
      + signal_address;
    printOnLog(LogLevel::ERROR,"MODAMODEM", err_msg);
    return;
  }

  if (!data_conn->openConnection(modem_address)) {
    std::string err_msg = "DATA_CHANNEL_FAILED_TO_OPEN_AT_PORT:"
      + modem_address;
    printOnLog(LogLevel::ERROR,"MODAMODEM", err_msg);
    return;
  }

  // set flags to true so loops can start
  receiving.store(true);
  transmitting.store(true);

  // Dispatch threads
  sig_thread = std::thread(&UwMODAModem::receivingSignaling, this);
  rx_thread = std::thread(&UwMODAModem::receivingData, this);
  tx_thread = std::thread(&UwMODAModem::transmittingData, this);

  checkTimer = new CheckTimer(this);
  checkTimer->resched(period);
}

void
UwMODAModem::stop()
{
  receiving.store(false);
  transmitting.store(false);

  status_cv.notify_all();
  tx_queue_cv.notify_all();

  if (tx_thread.joinable())
    tx_thread.join();

  if (signal_conn->isConnected() && !signal_conn->closeConnection())
    printOnLog(LogLevel::ERROR,
        "MODAMODEM", "SIGNAL_CONNECTION_UNABLE_TO_CLOSE");
  if (data_conn->isConnected() && !data_conn->closeConnection())
    printOnLog(LogLevel::ERROR,
        "MODAMODEM","DATA_CONNECTION_UNABLE_TO_CLOSE");

  if (sig_thread.joinable())
    sig_thread.join();
  if (rx_thread.joinable())
    rx_thread.join();

  checkTimer->force_cancel();
}

void
UwMODAModem::receivingData()
{
  data_buffer.reserve(DATA_BUFFER_LEN);
  std::fill(data_buffer.begin(), data_buffer.end(), '\0');

  while (receiving.load())
  {
    std::unique_lock<std::mutex> state_lock(status_m);
    if (status_cv.wait_for(state_lock, MODEM_TIMEOUT, [&] {
      return status == ModemState::RECEIVING;
    })) {

      state_lock.unlock();

      auto beg_it = data_buffer.begin();
      int r_bytes = data_conn->readFromDevice(&(*beg_it), MAX_READ_BYTES);

      if (r_bytes > 0) {

        while (r_bytes < rx_size) {
          r_bytes += data_conn->readFromDevice(&(*beg_it) + r_bytes,
              MAX_READ_BYTES - r_bytes);
        }

        rx_payload = std::string(beg_it, beg_it + r_bytes);
        printOnLog(LogLevel::DEBUG, "MODAMODEM", "receivingData::LEN::" +
          std::to_string(rx_payload.size()) + "::DATA::" + rx_payload);

        Packet *p = Packet::alloc();
        createRxPacket(p);
        std::function<void(UwModem &, Packet * p)> callback = &UwModem::recv;
        ModemEvent e = {callback, p};
        event_q.push(e);

        data_buffer.clear();

      }

      state_lock.lock();
      status = ModemState::AVAILABLE;

    }

  }
}

void
UwMODAModem::createRxPacket(Packet *p)
{
	hdr_uwal *uwalh = HDR_UWAL(p);
	uwalh->binPktLength() = rx_payload.size();
	std::memset(uwalh->binPkt(), 0, uwalh->binPktLength());
	std::copy(rx_payload.begin(), rx_payload.end(), uwalh->binPkt());
	HDR_CMN(p)->direction() = hdr_cmn::UP;
}

void
UwMODAModem::receivingSignaling()
{
  signal_buffer.reserve(DATA_BUFFER_LEN);
  std::fill(signal_buffer.begin(), signal_buffer.end(), '\0');

  ModemResponse rsp = ModemResponse::UNDEFINED;

  auto beg_it = signal_buffer.begin();
  auto end_it = beg_it;

  while (receiving.load())
  {
    int r_bytes = signal_conn->readFromDevice(&(*end_it),
                                             MAX_READ_BYTES - (end_it - beg_it));
    end_it = end_it + r_bytes;

    rsp = parseSignaling(end_it);

    while (receiving.load() && rsp == ModemResponse::UNDEFINED) {
      r_bytes += signal_conn->readFromDevice(&(*end_it), MAX_READ_BYTES-r_bytes);
      end_it += r_bytes;
      rsp = parseSignaling(end_it);
    }

    updateStatus(rsp);

    // signal_buffer.clear();
  }
}

void
UwMODAModem::transmittingData()
{
  while (transmitting.load()) {

    std::unique_lock<std::mutex> tx_lock(tx_queue_m);

    tx_queue_cv.wait(
        tx_lock, [&] { return !tx_queue.empty() || !transmitting; });

    if (!transmitting.load())
      break;

    Packet *pck = tx_queue.front();
    tx_queue.pop();
    tx_lock.unlock();
    if (pck) {
        startTx(pck);
    }

    printOnLog(LogLevel::DEBUG,
        "MODAMODEM",
        "transmittingData::BLOCKING_ON_NEXT_PACKET");
  }
}

UwMODAModem::ModemResponse
UwMODAModem::parseSignaling(std::vector<char>::iterator& end_it)
{
  auto beg_it = signal_buffer.begin();
  int offset = 0;
  ModemResponse rsp = ModemResponse::UNDEFINED;

  // Identify of the signaling is for driver (me)
  auto tag_it = std::search(beg_it, end_it, signal_tag.begin(), signal_tag.end());

  if (tag_it != end_it) {

    // Identify which info the signalinf brings
    for (uint i = 0; i < signaling_dict.size(); i++) {
      auto comm_it = std::search(beg_it,
                                 end_it,
                                 signaling_dict[i].first.begin(),
                                 signaling_dict[i].first.end());
      if (comm_it != end_it) {

        rsp = signaling_dict[i].second;

        // if the info is related to reception started, retrieve size of the data
        if (rsp == ModemResponse::RX_BEG) {
          auto size_beg = std::search(comm_it, end_it, sep.begin(), sep.end());
          auto size_end = std::search(comm_it, end_it,
                                      end_delim.begin(), end_delim.end());
          std::string size_s = std::string(size_beg+sep.size(), size_end);
          rx_size = std::stoi(size_s);

          offset = end_it - (size_end + end_delim.size());
          std::copy(size_end + end_delim.size(),
                    end_it,
                    signal_buffer.begin());
          end_it = beg_it + offset;
        }

      }

    }

  }

  return rsp;
}

void
UwMODAModem::updateStatus(ModemResponse response)
{
  std::lock_guard<std::mutex> status_lock(status_m);

  switch (response) {
    case ModemResponse::TX_END:
      status = ModemState::AVAILABLE;
      status_cv.notify_all();
      break;
    case ModemResponse::CFG_END:
      status = ModemState::AVAILABLE;
      status_cv.notify_all();
      break;
    case ModemResponse::RX_BEG:
      status = ModemState::RECEIVING;
      status_cv.notify_all();
      break;
    default:
      printOnLog(LogLevel::ERROR, "MODAMODEM", "updateStatus::UNKNOWN_RESPONSE");
      return;
  }

  printOnLog(LogLevel::DEBUG, "MODAMODEM", "updateStatus::TO::" +
             stateToString[status]);
}


void
UwMODAModem::startTx(Packet* p)
{
  hdr_uwal *uwalh = HDR_UWAL(p);
  std::string payload;
  payload.assign(uwalh->binPkt(), uwalh->binPktLength());

  std::unique_lock<std::mutex> state_lock(status_m);
  if (status_cv.wait_for(state_lock, MODEM_TIMEOUT, [&] {
      return status == ModemState::AVAILABLE;
    })) {

    status = ModemState::TRANSMITTING;
    state_lock.unlock();

    if ((data_conn->writeToDevice(payload)) < 0) {
        printOnLog(LogLevel::ERROR,
            "MODAMODEM",
            "startTx::FAIL_TO_WRITE_DATA_TO_DEVICE");
        return;
    }

    std::function<void(UwModem &, Packet * p)> callback =
        &UwModem::realTxEnded;
    ModemEvent e = {callback, p};
    event_q.push(e);

    printOnLog(LogLevel::INFO, "MODAMODEM", "startTx::PACKET_TRANSMITTED");

  }
}
