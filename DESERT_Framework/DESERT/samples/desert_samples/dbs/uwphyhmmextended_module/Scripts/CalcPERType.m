% Returns the PER category of val
function PERType = CalcPERType(val)
    if val < 0.09
        PERType = 1; %Good PER
    elseif (0.09 < val) && (val < 0.32)
        PERType = 2; %Medium PER
    else
        PERType = 3; %Bad PER
    end
end