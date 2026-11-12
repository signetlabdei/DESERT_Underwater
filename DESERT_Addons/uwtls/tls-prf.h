//
// Copyright (c) 2026 Regents of the SIGNET lab, University of Padova.
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

/**
 * @file tls-prf.h
 * @author Davide Costa
 * \version 1.0.0
 * \brief  Header of the class responsible for some SSL key derivation functions.
 */

#ifndef UWTLS_PRF_H
#define UWTLS_PRF_H

#include <cstddef>
#include <cstdint>
#include <vector>

#include <stdexcept>

#include <openssl/evp.h>
#include <openssl/kdf.h>

namespace key_derivation {

enum DerivationResult { OK = 0, PRF_ERROR = -1, PREMASTER_ERROR = -2, RANDOMS_ERROR = -3 };

/**
 * Derives the TLS 1.2 key block.
 *
 * This function internally:
 * 1. Uses the decrypted premaster secret.
 * 2. Derives the 48-byte TLS 1.2 master secret using label "master secret".
 * 3. Derives the requested key block using label "key expansion".
 *
 * The premaster parameter must contain the decrypted premaster
 * value.
 *
 * The master secret is derived as:
 * PRF(premaster, "master secret", client_random || server_random)
 *
 * The key block is derived as:
 * PRF(master_secret, "key expansion", server_random || client_random)
 *
 * @param key_block Output vector that will contain the derived key block.
 * @param premaster Pointer to the decrypted premaster value.
 * @param client_random Pointer to the 32-byte client random.
 * @param server_random Pointer to the 32-byte server random.
 * @param out_len Number of bytes to derive for the key block.
 *
 * @returns Result of the OpenSSL key block derivation.
 */
DerivationResult derive_tls12_key_block(std::vector<uint8_t>& key_block,
                                        const uint8_t premaster[48],
                                        std::size_t premaster_len,
                                        const uint8_t client_random[32],
                                        const uint8_t server_random[32],
                                        std::size_t out_len);

} // namespace key_derivation

#endif // UWTLS_PRF_H
