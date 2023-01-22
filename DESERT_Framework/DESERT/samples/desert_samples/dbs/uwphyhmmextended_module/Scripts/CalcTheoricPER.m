function TheoricPER = CalcTheoricPER(BER, PacketBits, BlockBits)
    PSuccSeq = (1-BER)^7 + 7*BER*(1-BER)^6;
    TheoricPER = 1 - (PSuccSeq)^(PacketBits/BlockBits);
end