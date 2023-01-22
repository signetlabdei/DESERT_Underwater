% Returns the BER category of val
function BERType = CalcBERType(val)
    if val < 0.012
        BERType = 1; %Good BER
    elseif (0.012 < val) && (val < 0.025)
        BERType = 2; %Medium BER
    else
        BERType = 3; %Bad BER
    end
end