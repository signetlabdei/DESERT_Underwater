% calculates the 3x3 matrix of transition probabilities (P) or the correponding
% "absolute freqs" matrix (I) between the 3
% states(good, medium, bad) (according to flag, which can be 'prob' or
% 'int') of a single link in a single topology
function P = CalcTransitionVec(V, flag, BP)
    P = zeros(3, 3);
    for i = 1:length(V)-1
        if strcmp(BP, 'BER')
            BERNow = CalcBERType(V(i));
            BERNext = CalcBERType(V(i+1));
            P(BERNow, BERNext) = P(BERNow, BERNext) + 1;
        else
            PERNow = CalcPERType(V(i));
            PERNext = CalcPERType(V(i+1));
            P(PERNow, PERNext) = P(PERNow, PERNext) + 1;
        end
    end
    if strcmp(flag, 'prob')
        SumRow1 = sum(P(1, :));
        SumRow2 = sum(P(2, :));
        SumRow3 = sum(P(3, :));
        P(1, :) = P(1, :)/SumRow1;
        P(2, :) = P(2, :)/SumRow2;
        P(3, :) = P(3, :)/SumRow3;
    else
        if ~strcmp(flag, 'int')
            disp('Flag is not valid');
        end
    end
end