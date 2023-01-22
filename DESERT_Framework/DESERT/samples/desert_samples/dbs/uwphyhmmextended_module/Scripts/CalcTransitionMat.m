% calculates a 3x3 transition probability matrix or the corresponding
% "absolute freqs" matrix (according to flag, which can be 'prob' or
% 'int') for each link in a
% topology and so returns a 3x3x4x4 matrix
function P = CalcTransitionMat(M, flag, BP)
P = zeros(3, 3, 4, 4);
for i = 1:4
    for j = 1:4
        if isnan(M(1, i, j))
            P(:, :, i, j) = NaN;
        else
            P(:, :, i, j) = CalcTransitionVec(M(:, i, j), flag, BP);
        end
    end
end