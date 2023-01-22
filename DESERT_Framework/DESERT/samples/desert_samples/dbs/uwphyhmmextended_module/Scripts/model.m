%% Data loading
load('HaifaHarborExp-05-09.mat')

%% AdjMat adjustment
for i = 1:4
    AdjMat1(:, i, i) = zeros(360, 1);
end

AdjMat2(:, 4, 2) = ones(720, 1);
AdjMat2(:, 2, 4) = ones(720, 1);

AdjMat6(:, 2, 1) = zeros(1080, 1);
AdjMat6(:, 1, 2) = zeros(1080, 1);

FullAdjMat = cat(1, AdjMat1, AdjMat2, AdjMat3, AdjMat4, AdjMat5, AdjMat6);
%% Tidying up of TopMats
TopNanMat1 = nan * ones(360, 4, 4);
TopNanMat2 = nan * ones(720, 4, 4);
TopNanMat3 = nan * ones(720, 4, 4);
TopNanMat4 = nan * ones(720, 4, 4);
TopNanMat5 = nan * ones(720, 4, 4);
TopNanMat6 = nan * ones(1080, 4, 4);

for i = 1:360
    for j = 1:4
        for k = 1:4
            if AdjMat1(i, j, k) == 1
                TopNanMat1(i, j, k) = TopMat1(i, j, k);
            end
        end
    end
end

for i = 1:720
    for j = 1:4
        for k = 1:4
            if AdjMat2(i, j, k) == 1
                TopNanMat2(i, j, k) = TopMat2(i, j, k);
            end
        end
    end
end

for i = 1:720
    for j = 1:4
        for k = 1:4
            if AdjMat3(i, j, k) == 1
                TopNanMat3(i, j, k) = TopMat3(i, j, k);
            end
        end
    end
end

for i = 1:720
    for j = 1:4
        for k = 1:4
            if AdjMat4(i, j, k) == 1
                TopNanMat4(i, j, k) = TopMat4(i, j, k);
            end
        end
    end
end

for i = 1:720
    for j = 1:4
        for k = 1:4
            if AdjMat5(i, j, k) == 1
                TopNanMat5(i, j, k) = TopMat5(i, j, k);
            end
        end
    end
end

for i = 1:1080
    for j = 1:4
        for k = 1:4
            if AdjMat6(i, j, k) == 1
                TopNanMat6(i, j, k) = TopMat6(i, j, k);
            end
        end
    end
end

%% Transition probabilities matrices
% the 2D matrix TransMatk(:,:,i,j) is the transition probability matrix of
% link i-j in topology k (can be for BER awa for PER)
TransProbMat1 = CalcTransitionMat(TopNanMat1, 'prob', 'BER');
TransProbMat2 = CalcTransitionMat(TopNanMat2, 'prob', 'BER');
TransProbMat3 = CalcTransitionMat(TopNanMat3, 'prob', 'BER');
TransProbMat4 = CalcTransitionMat(TopNanMat4, 'prob', 'BER');
TransProbMat5 = CalcTransitionMat(TopNanMat5, 'prob', 'BER');
TransProbMat6 =CalcTransitionMat(TopNanMat6, 'prob', 'BER');

TransIntMat1 = CalcTransitionMat(TopNanMat1, 'int', 'BER');
TransIntMat2 = CalcTransitionMat(TopNanMat2, 'int', 'BER');
TransIntMat3 = CalcTransitionMat(TopNanMat3, 'int', 'BER');
TransIntMat4 = CalcTransitionMat(TopNanMat4, 'int', 'BER');
TransIntMat5 = CalcTransitionMat(TopNanMat5, 'int', 'BER');
TransIntMat6 =CalcTransitionMat(TopNanMat6, 'int', 'BER');

%% confusion charts
ConfusionTopology = input('Choose topology 1-6 to disp the corresponding confusion matrices:  ');
switch ConfusionTopology
    
    case 1
        
        for i = 1:4
            for j = 1:4
                if sum(isnan(TransIntMat1(:,:,i,j))) %NaN in data! Can't plot confusion matrix
                    continue;
                else
                    figure();
                    cm = confusionchart(TransIntMat1(:,:,i,j), {'Good','Medium','Bad'}, 'YLabel', 'Previous State', 'XLabel',...
                        'Actual State', 'Normalization', 'row-normalized', 'FontSize', 16, 'DiagonalColor', [0.8500 0.3250 0.0980],...
                        'OffDiagonalColor', [0.8500 0.3250 0.0980]);
                    title = ['Transition matrix link ' num2str(i) '-' num2str(j) ', Topology 1'];
                    cm.Title = title;
                    sortClasses(cm, ["Good", "Medium", "Bad"]);
                end
            end
        end
        
    case 2
        
        for i = 1:4
            for j = 1:4
                if sum(isnan(TransIntMat2(:,:,i,j))) %NaN in data! Can't plot confusion matrix
                    continue;
                else
                    figure();
                    cm = confusionchart(TransIntMat2(:,:,i,j), {'Good','Medium','Bad'}, 'YLabel', 'Previous State', 'XLabel',...
                        'Actual State', 'Normalization', 'row-normalized', 'FontSize', 16, 'DiagonalColor', [0.8500 0.3250 0.0980],...
                        'OffDiagonalColor', [0.8500 0.3250 0.0980]);
                    title = ['Transition matrix link ' num2str(i) '-' num2str(j) ', Topology 2'];
                    cm.Title = title;
                    sortClasses(cm, ["Good", "Medium", "Bad"]);
                end
            end
        end
        
    case 3
        
        for i = 1:4
            for j = 1:4
                if sum(isnan(TransIntMat3(:,:,i,j))) %NaN in data! Can't plot confusion matrix
                    continue;
                else
                    figure();
                    cm = confusionchart(TransIntMat3(:,:,i,j), {'Good','Medium','Bad'}, 'YLabel', 'Previous State', 'XLabel',...
                        'Actual State', 'Normalization', 'row-normalized', 'FontSize', 16, 'DiagonalColor', [0.8500 0.3250 0.0980],...
                        'OffDiagonalColor', [0.8500 0.3250 0.0980]);
                    title = ['Transition matrix link ' num2str(i) '-' num2str(j) ', Topology 3'];
                    cm.Title = title;
                    sortClasses(cm, ["Good", "Medium", "Bad"]);
                end
            end
        end
        
    case 4
        
        for i = 1:4
            for j = 1:4
                if sum(isnan(TransIntMat4(:,:,i,j))) %NaN in data! Can't plot confusion matrix
                    continue;
                else
                    figure();
                    cm = confusionchart(TransIntMat4(:,:,i,j), {'Good','Medium','Bad'}, 'YLabel', 'Previous State', 'XLabel',...
                        'Actual State', 'Normalization', 'row-normalized', 'FontSize', 16, 'DiagonalColor', [0.8500 0.3250 0.0980],...
                        'OffDiagonalColor', [0.8500 0.3250 0.0980]);
                    title = ['Transition matrix link ' num2str(i) '-' num2str(j) ', Topology 4'];
                    cm.Title = title;
                    sortClasses(cm, ["Good", "Medium", "Bad"]);
                end
            end
        end
        
    case 5
        
        for i = 1:4
            for j = 1:4
                if sum(isnan(TransIntMat5(:,:,i,j))) %NaN in data! Can't plot confusion matrix
                    continue;
                else
                    figure();
                    cm = confusionchart(TransIntMat5(:,:,i,j), {'Good','Medium','Bad'}, 'YLabel', 'Previous State', 'XLabel',...
                        'Actual State', 'Normalization', 'row-normalized', 'FontSize', 16, 'DiagonalColor', [0.8500 0.3250 0.0980],...
                        'OffDiagonalColor', [0.8500 0.3250 0.0980]);
                    title = ['Transition matrix link ' num2str(i) '-' num2str(j) ', Topology 5'];
                    cm.Title = title;
                    sortClasses(cm, ["Good", "Medium", "Bad"]);
                end
            end
        end
        
    case 6
        
        for i = 1:4
            for j = 1:4
                if sum(isnan(TransIntMat6(:,:,i,j))) %NaN in data! Can't plot confusion matrix
                    continue;
                else
                    figure();
                    cm = confusionchart(TransIntMat6(:,:,i,j), {'Good','Medium','Bad'}, 'YLabel', 'Previous State', 'XLabel',...
                        'Actual State', 'Normalization', 'row-normalized', 'FontSize', 16, 'DiagonalColor', [0.8500 0.3250 0.0980],...
                        'OffDiagonalColor', [0.8500 0.3250 0.0980]);
                    title = ['Transition matrix link ' num2str(i) '-' num2str(j) ', Topology 6'];
                    cm.Title = title;
                    sortClasses(cm, ["Good", "Medium", "Bad"]);
                end
            end
        end
        
    otherwise
        disp('Choose a valid Topology number');
end

%% Fit the data (linkwise) to exponential distributions and get the probability of having a good/medium/bad channel
clear title;
%Topology 1
ChannelTypeProb1 = nan*ones(3, 4, 4); %to store channel type probabilities for every link
for i = 1:4
    for j = 1:4
        if sum(isnan(TopNanMat1(:, i, j))) %NaN in data! Can't fit an appropriate distribution
            continue;
        else
            data_vec = TopNanMat1(:, i, j);
            
            pde = fitdist(data_vec, 'exponential');
            P_good = cdf(pde, 0.012);
            P_medium = cdf(pde, 0.025) - P_good;
            P_bad = 1 - P_medium - P_good;
            
            ChannelTypeProb1(:, i, j) = [P_good, P_medium, P_bad];
            %uncomment to get plots for CDF fits for this topology
            %{
            dt_bins = linspace(min(data_vec), max(data_vec), 100);
            Fy_exp = cdf('exponential', dt_bins, pde.mu);
            fig = figure();
            ax = axes('Parent', fig);
            hold (ax, 'on');
            rectangle('Parent', ax, 'Position', [0 0 0.012 1], 'FaceColor', [0 1 0 0.3], 'LineStyle', 'none');
            rectangle('Parent', ax, 'Position', [0.012 0 0.013 1], 'FaceColor', [0.9290 0.6940 0.1250 0.3], 'LineStyle', 'none');
            if max(TopNanMat1(:, i, j)) - 0.025 > 0
                rectangle('Parent', ax, 'Position', [0.025 0 max(TopNanMat1(:, i, j))-0.025 1], 'FaceColor', [0.8500 0.3250 0.0980 0.3], 'LineStyle', 'none');
            end
            cdfplot(data_vec);
            plot(dt_bins, Fy_exp);
            name = ['CDF fit for link ' num2str(i) '-' num2str(j) ', Topology 1'];
            title(name);
            legend('Empirical', 'Exponential fit');
            ylim([0 1]);
            xlim([0 max(TopNanMat1(:, i, j))]);
            %}
            
        end
    end
end

%Topology 2
ChannelTypeProb2 = nan*ones(3, 4, 4); %to store channel type probabilities for every link
for i = 1:4
    for j = 1:4
        if sum(isnan(TopNanMat2(:, i, j))) %NaN in data! Can't fit an appropriate distribution
            continue;
        else
            data_vec = TopNanMat2(:, i, j);
            
            pde = fitdist(data_vec, 'exponential');
            P_good = cdf(pde, 0.012);
            P_medium = cdf(pde, 0.025) - P_good;
            P_bad = 1 - P_medium - P_good;
            
            ChannelTypeProb2(:, i, j) = [P_good, P_medium, P_bad];
            %uncomment to get plots for CDF fits for this topology
            %{
            dt_bins = linspace(min(data_vec), max(data_vec), 100);
            Fy_exp = cdf('exponential', dt_bins, pde.mu);
            fig = figure();
            ax = axes('Parent', fig);
            hold (ax, 'on');
            rectangle('Parent', ax, 'Position', [0 0 0.012 1], 'FaceColor', [0 1 0 0.3], 'LineStyle', 'none');
            rectangle('Parent', ax, 'Position', [0.012 0 0.013 1], 'FaceColor', [0.9290 0.6940 0.1250 0.3], 'LineStyle', 'none');
            if max(TopNanMat2(:, i, j)) - 0.025 > 0
                rectangle('Parent', ax, 'Position', [0.025 0 max(TopNanMat2(:, i, j))-0.025 1], 'FaceColor', [0.8500 0.3250 0.0980 0.3], 'LineStyle', 'none');
            end
            cdfplot(data_vec);
            plot(dt_bins, Fy_exp);
            name = ['CDF fit for link ' num2str(i) '-' num2str(j) ', Topology 2'];
            title(name);
            legend('Empirical', 'Exponential fit');
            ylim([0 1]);
            xlim([0 max(TopNanMat2(:, i, j))]);
            %}
            
        end
    end
end

%Topology 3
ChannelTypeProb3 = nan*ones(3, 4, 4); %to store channel type probabilities for every link
for i = 1:4
    for j = 1:4
        if sum(isnan(TopNanMat3(:, i, j))) %NaN in data! Can't fit an appropriate distribution
            continue;
        else
            data_vec = TopNanMat3(:, i, j);
            
            pde = fitdist(data_vec, 'exponential');
            P_good = cdf(pde, 0.012);
            P_medium = cdf(pde, 0.025) - P_good;
            P_bad = 1 - P_medium - P_good;
            
            ChannelTypeProb3(:, i, j) = [P_good, P_medium, P_bad];
            %uncomment to get plots for CDF fits for this topology
            %{
            dt_bins = linspace(min(data_vec), max(data_vec), 100);
            Fy_exp = cdf('exponential', dt_bins, pde.mu);
            fig = figure();
            ax = axes('Parent', fig);
            hold (ax, 'on');
            rectangle('Parent', ax, 'Position', [0 0 0.012 1], 'FaceColor', [0 1 0 0.3], 'LineStyle', 'none');
            rectangle('Parent', ax, 'Position', [0.012 0 0.013 1], 'FaceColor', [0.9290 0.6940 0.1250 0.3], 'LineStyle', 'none');
            if max(TopNanMat3(:, i, j)) - 0.025 > 0
                rectangle('Parent', ax, 'Position', [0.025 0 max(TopNanMat3(:, i, j))-0.025 1], 'FaceColor', [0.8500 0.3250 0.0980 0.3], 'LineStyle', 'none');
            end
            cdfplot(data_vec);
            plot(dt_bins, Fy_exp);
            name = ['CDF fit for link ' num2str(i) '-' num2str(j) ', Topology 3'];
            title(name);
            legend('Empirical', 'Exponential fit');
            ylim([0 1]);
            xlim([0 max(TopNanMat3(:, i, j))]);
            %}
            
        end
    end
end

%Topology 4
ChannelTypeProb4 = nan*ones(3, 4, 4); %to store channel type probabilities for every link
for i = 1:4
    for j = 1:4
        if sum(isnan(TopNanMat4(:, i, j))) %NaN in data! Can't fit an appropriate distribution
            continue;
        else
            data_vec = TopNanMat4(:, i, j);
            
            pde = fitdist(data_vec, 'exponential');
            P_good = cdf(pde, 0.012);
            P_medium = cdf(pde, 0.025) - P_good;
            P_bad = 1 - P_medium - P_good;
            
            ChannelTypeProb4(:, i, j) = [P_good, P_medium, P_bad];
            %uncomment to get plots for CDF fits for this topology
            %{
            dt_bins = linspace(min(data_vec), max(data_vec), 100);
            Fy_exp = cdf('exponential', dt_bins, pde.mu);
            fig = figure();
            ax = axes('Parent', fig);
            hold (ax, 'on');
            rectangle('Parent', ax, 'Position', [0 0 0.012 1], 'FaceColor', [0 1 0 0.3], 'LineStyle', 'none');
            rectangle('Parent', ax, 'Position', [0.012 0 0.013 1], 'FaceColor', [0.9290 0.6940 0.1250 0.3], 'LineStyle', 'none');
            if max(TopNanMat4(:, i, j)) - 0.025 > 0
                rectangle('Parent', ax, 'Position', [0.025 0 max(TopNanMat4(:, i, j))-0.025 1], 'FaceColor', [0.8500 0.3250 0.0980 0.3], 'LineStyle', 'none');
            end
            cdfplot(data_vec);
            plot(dt_bins, Fy_exp);
            name = ['CDF fit for link ' num2str(i) '-' num2str(j) ', Topology 4'];
            title(name);
            legend('Empirical', 'Exponential fit');
            ylim([0 1]);
            xlim([0 max(TopNanMat4(:, i, j))]);
            %}
            
        end
    end
end

%Topology 5
ChannelTypeProb5 = nan*ones(3, 4, 4); %to store channel type probabilities for every link
for i = 1:4
    for j = 1:4
        if sum(isnan(TopNanMat5(:, i, j))) %NaN in data! Can't fit an appropriate distribution
            continue;
        else
            data_vec = TopNanMat5(:, i, j);
            
            pde = fitdist(data_vec, 'exponential');
            P_good = cdf(pde, 0.012);
            P_medium = cdf(pde, 0.025) - P_good;
            P_bad = 1 - P_medium - P_good;
            
            ChannelTypeProb5(:, i, j) = [P_good, P_medium, P_bad];
            %uncomment to get plots for CDF fits for this topology
            %{
            dt_bins = linspace(min(data_vec), max(data_vec), 100);
            Fy_exp = cdf('exponential', dt_bins, pde.mu);
            fig = figure();
            ax = axes('Parent', fig);
            hold (ax, 'on');
            rectangle('Parent', ax, 'Position', [0 0 0.012 1], 'FaceColor', [0 1 0 0.3], 'LineStyle', 'none');
            rectangle('Parent', ax, 'Position', [0.012 0 0.013 1], 'FaceColor', [0.9290 0.6940 0.1250 0.3], 'LineStyle', 'none');
            if max(TopNanMat5(:, i, j)) - 0.025 > 0
                rectangle('Parent', ax, 'Position', [0.025 0 max(TopNanMat5(:, i, j))-0.025 1], 'FaceColor', [0.8500 0.3250 0.0980 0.3], 'LineStyle', 'none');
            end
            cdfplot(data_vec);
            plot(dt_bins, Fy_exp);
            name = ['CDF fit for link ' num2str(i) '-' num2str(j) ', Topology 5'];
            title(name);
            legend('Empirical', 'Exponential fit');
            ylim([0 1]);
            xlim([0 max(TopNanMat5(:, i, j))]);
            %}
            
        end
    end
end

%Topology 6
ChannelTypeProb6 = nan*ones(3, 4, 4); %to store channel type probabilities for every link
for i = 1:4
    for j = 1:4
        if sum(isnan(TopNanMat6(:, i, j))) %NaN in data! Can't fit an appropriate distribution
            continue;
        else
            data_vec = TopNanMat6(:, i, j);
            
            pde = fitdist(data_vec, 'exponential');
            P_good = cdf(pde, 0.012);
            P_medium = cdf(pde, 0.025) - P_good;
            P_bad = 1 - P_medium - P_good;
            
            ChannelTypeProb6(:, i, j) = [P_good, P_medium, P_bad];
            %uncomment to get plots for CDF fits for this topology
            %{
            dt_bins = linspace(min(data_vec), max(data_vec), 100);
            Fy_exp = cdf('exponential', dt_bins, pde.mu);
            fig = figure();
            ax = axes('Parent', fig);
            hold (ax, 'on');
            rectangle('Parent', ax, 'Position', [0 0 0.012 1], 'FaceColor', [0 1 0 0.3], 'LineStyle', 'none');
            rectangle('Parent', ax, 'Position', [0.012 0 0.013 1], 'FaceColor', [0.9290 0.6940 0.1250 0.3], 'LineStyle', 'none');
            if max(TopNanMat6(:, i, j)) - 0.025 > 0
                rectangle('Parent', ax, 'Position', [0.025 0 max(TopNanMat6(:, i, j))-0.025 1], 'FaceColor', [0.8500 0.3250 0.0980 0.3], 'LineStyle', 'none');
            end
            cdfplot(data_vec);
            plot(dt_bins, Fy_exp);
            name = ['CDF fit for link ' num2str(i) '-' num2str(j) ', Topology 6'];
            title(name);
            legend('Empirical', 'Exponential fit');
            ylim([0 1]);
            xlim([0 max(TopNanMat6(:, i, j))]);
            %}
            
        end
    end
end

%% Now try to find the PER with Hamming(7,4) FEC
%for every 4 bits of data we have 3 parity bits, thus we can correct up to
%one error for every 4 bits group of data - that is, the codeword and the
%received word have an Hamming distance that is not greater than one.
%We have pcks of 10 Byte = 80 bits each, so that every packet carries 80
%bits of data and 60 bits for FEC.
%We consider block of seven bits, with this structure:
%-------------------- p1-p2-x3-p4-x2-x1-x0 -----------------------
%where
%                        p1 = x3 XOR x2 XOR x0
%                        p2 = x3 XOR x1 XOR x0
%                        p4 = x2 XOR x1 XOR x0

%and we look at the c to know *the position of an error*: if c1c2c3 != 000
%then for sure a bit has been flipped in position (c1c2c3)_base10

%                        c1 = p1 XOR x3 XOR x2 XOR x0
%                        c2 = p2 XOR x3 XOR x1 XOR x0
%                        c4 = p4 XOR x2 XOR x1 XOR x0

%we know the bps (600) and the duration of the experiments. Thus, we can
%simulate the PER at the instant time t by analyzing sequence of 140 bits
%(in an epoch we 1000 pcks). In a
%sequence of 140 bits (i.e. a packet), we look for contiguous series of 7
%bits - the block length of Hamming 7,4. If in a sequence there is more
%than one error, the sequence, and therefore the whole packet, is
%considered corrupted. '0' = correct bit, '1' = wrong bit
BitsInPck = 224;
NumTxPck = 100; % >> 21 pcks in real experiment

%Topology 1
PERNanMat1 = TopNanMat1;
for i = 1:4
    for j = 1:4
        for t = 1:length(TopNanMat1)
            if isnan(TopNanMat1(t, i, j)) %NaN in data! Skip
                continue;
            else
                NumWrongPck = 0;
                for p = 1:NumTxPck
                    
                    r = rand(BitsInPck,1); %uniform sequence of BER 140 bits
                    x = r < TopNanMat1(t, i, j);
                    
                    k = 1;
                    while k <= BitsInPck
                        if sum(x(k:k+6)) > 1 %there's more than one error in the sequence
                            NumWrongPck = NumWrongPck + 1; %the whole pck is corrupted
                            break;
                        end
                        k = k+7;
                    end
                    
                end
            end
            PERNanMat1(t, i, j) = NumWrongPck/NumTxPck;
        end
    end
end

%Topology 2
PERNanMat2 = TopNanMat2;
for i = 1:4
    for j = 1:4
        for t = 1:length(TopNanMat2)
            if isnan(TopNanMat2(t, i, j)) %NaN in data! Skip
                continue;
            else
                NumWrongPck = 0;
                for p = 1:NumTxPck
                    
                    r = rand(BitsInPck,1); %uniform sequence of BER 140 bits
                    x = r < TopNanMat2(t, i, j);
                    
                    k = 1;
                    while k <= BitsInPck
                        if sum(x(k:k+6)) > 1 %there's more than one error in the sequence
                            NumWrongPck = NumWrongPck + 1; %the whole pck is corrupted
                            break;
                        end
                        k = k+7;
                    end
                    
                end
            end
            PERNanMat2(t, i, j) = NumWrongPck/NumTxPck;
        end
    end
end

%Topology 3
PERNanMat3 = TopNanMat3;
for i = 1:4
    for j = 1:4
        for t = 1:length(TopNanMat3)
            if isnan(TopNanMat3(t, i, j)) %NaN in data! Skip
                continue;
            else
                NumWrongPck = 0;
                for p = 1:NumTxPck
                    
                    r = rand(BitsInPck,1); %uniform sequence of BER 140 bits
                    x = r < TopNanMat3(t, i, j);
                    
                    k = 1;
                    while k <= BitsInPck
                        if sum(x(k:k+6)) > 1 %there's more than one error in the sequence
                            NumWrongPck = NumWrongPck + 1; %the whole pck is corrupted
                            break;
                        end
                        k = k+7;
                    end
                    
                end
            end
            PERNanMat3(t, i, j) = NumWrongPck/NumTxPck;
        end
    end
end

%Topology 4
PERNanMat4 = TopNanMat4;
for i = 1:4
    for j = 1:4
        for t = 1:length(TopNanMat4)
            if isnan(TopNanMat4(t, i, j)) %NaN in data! Skip
                continue;
            else
                NumWrongPck = 0;
                for p = 1:NumTxPck
                    
                    r = rand(BitsInPck,1); %uniform sequence of BER 140 bits
                    x = r < TopNanMat4(t, i, j);
                    
                    k = 1;
                    while k <= BitsInPck
                        if sum(x(k:k+6)) > 1 %there's more than one error in the sequence
                            NumWrongPck = NumWrongPck + 1; %the whole pck is corrupted
                            break;
                        end
                        k = k+7;
                    end
                    
                end
            end
            PERNanMat4(t, i, j) = NumWrongPck/NumTxPck;
        end
    end
end

%Topology 5
PERNanMat5 = TopNanMat5;
for i = 1:4
    for j = 1:4
        for t = 1:length(TopNanMat5)
            if isnan(TopNanMat5(t, i, j)) %NaN in data! Skip
                continue;
            else
                NumWrongPck = 0;
                for p = 1:NumTxPck
                    
                    r = rand(BitsInPck,1); %uniform sequence of BER 140 bits
                    x = r < TopNanMat5(t, i, j);
                    
                    k = 1;
                    while k <= BitsInPck
                        if sum(x(k:k+6)) > 1 %there's more than one error in the sequence
                            NumWrongPck = NumWrongPck + 1; %the whole pck is corrupted
                            break;
                        end
                        k = k+7;
                    end
                    
                end
            end
            PERNanMat5(t, i, j) = NumWrongPck/NumTxPck;
        end
    end
end

%Topology 6
PERNanMat6 = TopNanMat6;
for i = 1:4
    for j = 1:4
        for t = 1:length(TopNanMat6)
            if isnan(TopNanMat6(t, i, j)) %NaN in data! Skip
                continue;
            else
                NumWrongPck = 0;
                for p = 1:NumTxPck
                    
                    r = rand(BitsInPck,1); %uniform sequence of BER 140 bits
                    x = r < TopNanMat6(t, i, j);
                    
                    k = 1;
                    while k <= BitsInPck
                        if sum(x(k:k+6)) > 1 %there's more than one error in the sequence
                            NumWrongPck = NumWrongPck + 1; %the whole pck is corrupted
                            break;
                        end
                        k = k+7;
                    end
                    
                end
            end
            PERNanMat6(t, i, j) = NumWrongPck/NumTxPck;
        end
    end
end
%% Find theoretic PER with same conditions as before
BlockLength = 7;
BitsInPck = 224;

%Topology 1
TheoPERNanMat1 = TopNanMat1;
for i = 1:4
    for j = 1:4
        for t = 1:length(TopNanMat1)
            if isnan(TopNanMat1(t, i, j)) %NaN in data! Skip
                continue;
            else
                TheoPERNanMat1(t, i, j) = CalcTheoricPER(TopNanMat1(t, i, j), BitsInPck, BlockLength);
            end
        end
    end
end

%Topology 2
TheoPERNanMat2 = TopNanMat2;
for i = 1:4
    for j = 1:4
        for t = 1:length(TopNanMat2)
            if isnan(TopNanMat2(t, i, j)) %NaN in data! Skip
                continue;
            else
                TheoPERNanMat2(t, i, j) = CalcTheoricPER(TopNanMat2(t, i, j), BitsInPck, BlockLength);
            end
        end
    end
end

%Topology 3
TheoPERNanMat3 = TopNanMat3;
for i = 1:4
    for j = 1:4
        for t = 1:length(TopNanMat3)
            if isnan(TopNanMat3(t, i, j)) %NaN in data! Skip
                continue;
            else
                TheoPERNanMat3(t, i, j) = CalcTheoricPER(TopNanMat3(t, i, j), BitsInPck, BlockLength);
            end
        end
    end
end

%Topology 4
TheoPERNanMat4 = TopNanMat4;
for i = 1:4
    for j = 1:4
        for t = 1:length(TopNanMat4)
            if isnan(TopNanMat4(t, i, j)) %NaN in data! Skip
                continue;
            else
                TheoPERNanMat4(t, i, j) = CalcTheoricPER(TopNanMat4(t, i, j), BitsInPck, BlockLength);
            end
        end
    end
end

%Topology 5
TheoPERNanMat5 = TopNanMat5;
for i = 1:4
    for j = 1:4
        for t = 1:length(TopNanMat5)
            if isnan(TopNanMat5(t, i, j)) %NaN in data! Skip
                continue;
            else
                TheoPERNanMat5(t, i, j) = CalcTheoricPER(TopNanMat5(t, i, j), BitsInPck, BlockLength);
            end
        end
    end
end

%Topology 6
TheoPERNanMat6 = TopNanMat6;
for i = 1:4
    for j = 1:4
        for t = 1:length(TopNanMat6)
            if isnan(TopNanMat6(t, i, j)) %NaN in data! Skip
                continue;
            else
                TheoPERNanMat6(t, i, j) = CalcTheoricPER(TopNanMat6(t, i, j), BitsInPck, BlockLength);
            end
        end
    end
end

%% Plot PER vs BER for a given topology and a given link (both theretic and experimental results)
clear title;
Topology = input('Choose topology 1-6 :  ');
%i = input('Choose transmitting node 1-4 :  ');
%j = input('Choose receiving node 1-4 :  ');

switch Topology
    
    case 1
        for i = 1:4
            for j = 1:4
                if isnan(sum(TopNanMat1(:, i, j)))
                    disp('NaN in data; the link may not exist');
                else
                    fig = figure();
                    ax = axes('Parent', fig);
                    hold (ax, 'on');
                    
                    rectangle('Parent', ax, 'Position', [0 0 0.012 1], 'FaceColor', [0 1 0 0.3], 'LineStyle', 'none');
                    rectangle('Parent', ax, 'Position', [0.012 0 0.013 1], 'FaceColor', [0.9290 0.6940 0.1250 0.3], 'LineStyle', 'none');
                    if max(TopNanMat1(:, i, j)) - 0.025 > 0
                        rectangle('Parent', ax, 'Position', [0.025 0 max(TopNanMat1(:, i, j))-0.025 1], 'FaceColor', [0.8500 0.3250 0.0980 0.3], 'LineStyle', 'none');
                    end
                    
                    f = fit(TopNanMat1(:, i, j), TheoPERNanMat1(:, i, j), 'smoothingspline');
                    plot(TopNanMat1(:, i, j), PERNanMat1(:, i, j),  'xb', 'MarkerSize', 8);
                    plot(TopNanMat1(:, i, j), TheoPERNanMat1(:, i, j),  'or', 'MarkerSize', 3, 'MarkerFaceColor','r');
                    plot(f, '-r');
                    legend('Simulation data', 'Theoretical data', 'Fitting curve');
                    
                    name = ['PER vs BER in Topology 1, link ' num2str(i) '-' num2str(j)];
                    title(name);
                    lab = ['PER (pck size is ' num2str(BitsInPck) ' bits (including FEC))'];
                    ylabel(lab);
                    xlabel('BER');
                    ylim([0 1]);
                    xlim([0 max(TopNanMat1(:, i, j))]);
                end
            end
        end
        
    case 2
        for i = 1:4
            for j = 1:4
                if isnan(sum(TopNanMat2(:, i, j)))
                    disp('NaN in data; the link may not exist');
                else
                    fig = figure();
                    ax = axes('Parent', fig);
                    hold (ax, 'on');
                    
                    rectangle('Parent', ax, 'Position', [0 0 0.012 1], 'FaceColor', [0 1 0 0.3], 'LineStyle', 'none');
                    rectangle('Parent', ax, 'Position', [0.012 0 0.013 1], 'FaceColor', [0.9290 0.6940 0.1250 0.3], 'LineStyle', 'none');
                    if max(TopNanMat2(:, i, j)) - 0.025 > 0
                        rectangle('Parent', ax, 'Position', [0.025 0 max(TopNanMat2(:, i, j))-0.025 1], 'FaceColor', [0.8500 0.3250 0.0980 0.3], 'LineStyle', 'none');
                    end
                    
                    f = fit(TopNanMat2(:, i, j), TheoPERNanMat2(:, i, j), 'smoothingspline');
                    plot(TopNanMat2(:, i, j), PERNanMat2(:, i, j),  'xb', 'MarkerSize', 8);
                    plot(TopNanMat2(:, i, j), TheoPERNanMat2(:, i, j),  'or', 'MarkerSize', 3, 'MarkerFaceColor','r');
                    plot(f, '-r');
                    legend('Simulation data', 'Theoretical data', 'Fitting curve');
                    
                    name = ['PER vs BER in Topology 2, link ' num2str(i) '-' num2str(j)];
                    title(name);
                    lab = ['PER (pck size is ' num2str(BitsInPck) ' bits (including FEC))'];
                    ylabel(lab);
                    xlabel('BER');
                    ylim([0 1]);
                    xlim([0 max(TopNanMat2(:, i, j))]);
                end
            end
        end
        
    case 3
        for i = 1:4
            for j = 1:4
                if isnan(sum(TopNanMat3(:, i, j)))
                    disp('NaN in data; the link may not exist');
                else
                    fig = figure();
                    ax = axes('Parent', fig);
                    hold (ax, 'on');
                    
                    rectangle('Parent', ax, 'Position', [0 0 0.012 1], 'FaceColor', [0 1 0 0.3], 'LineStyle', 'none');
                    rectangle('Parent', ax, 'Position', [0.012 0 0.013 1], 'FaceColor', [0.9290 0.6940 0.1250 0.3], 'LineStyle', 'none');
                    if max(TopNanMat3(:, i, j)) - 0.025 > 0
                        rectangle('Parent', ax, 'Position', [0.025 0 max(TopNanMat3(:, i, j))-0.025 1], 'FaceColor', [0.8500 0.3250 0.0980 0.3], 'LineStyle', 'none');
                    end
                    
                    f = fit(TopNanMat3(:, i, j), TheoPERNanMat3(:, i, j), 'smoothingspline');
                    plot(TopNanMat3(:, i, j), PERNanMat3(:, i, j),  'xb', 'MarkerSize', 8);
                    plot(TopNanMat3(:, i, j), TheoPERNanMat3(:, i, j),  'or', 'MarkerSize', 3, 'MarkerFaceColor','r');
                    plot(f, '-r');
                    legend('Simulation data', 'Theoretical data', 'Fitting curve');
                    
                    name = ['PER vs BER in Topology 3, link ' num2str(i) '-' num2str(j)];
                    title(name);
                    lab = ['PER (pck size is ' num2str(BitsInPck) ' bits (including FEC))'];
                    ylabel(lab);
                    xlabel('BER');
                    ylim([0 1]);
                    xlim([0 max(TopNanMat3(:, i, j))]);
                end
            end
        end
        
    case 4
        for i = 1:4
            for j = 1:4
                if isnan(sum(TopNanMat4(:, i, j)))
                    disp('NaN in data; the link may not exist');
                else
                    fig = figure();
                    ax = axes('Parent', fig);
                    hold (ax, 'on');
                    
                    rectangle('Parent', ax, 'Position', [0 0 0.012 1], 'FaceColor', [0 1 0 0.3], 'LineStyle', 'none');
                    rectangle('Parent', ax, 'Position', [0.012 0 0.013 1], 'FaceColor', [0.9290 0.6940 0.1250 0.3], 'LineStyle', 'none');
                    if max(TopNanMat4(:, i, j)) - 0.025 > 0
                        rectangle('Parent', ax, 'Position', [0.025 0 max(TopNanMat4(:, i, j))-0.025 1], 'FaceColor', [0.8500 0.3250 0.0980 0.3], 'LineStyle', 'none');
                    end
                    
                    f = fit(TopNanMat4(:, i, j), TheoPERNanMat4(:, i, j), 'smoothingspline');
                    plot(TopNanMat4(:, i, j), PERNanMat4(:, i, j),  'xb', 'MarkerSize', 8);
                    plot(TopNanMat4(:, i, j), TheoPERNanMat4(:, i, j),  'or', 'MarkerSize', 3, 'MarkerFaceColor','r');
                    plot(f, '-r');
                    legend('Simulation data', 'Theoretical data', 'Fitting curve');
                    
                    name = ['PER vs BER in Topology 4, link ' num2str(i) '-' num2str(j)];
                    title(name);
                    lab = ['PER (pck size is ' num2str(BitsInPck) ' bits (including FEC))'];
                    ylabel(lab);
                    xlabel('BER');
                    ylim([0 1]);
                    xlim([0 max(TopNanMat4(:, i, j))]);
                end
            end
        end
        
    case 5
        for i = 1:4
            for j = 1:4
                if isnan(sum(TopNanMat5(:, i, j)))
                    disp('NaN in data; the link may not exist');
                else
                    fig = figure();
                    ax = axes('Parent', fig);
                    hold (ax, 'on');
                    
                    rectangle('Parent', ax, 'Position', [0 0 0.012 1], 'FaceColor', [0 1 0 0.3], 'LineStyle', 'none');
                    rectangle('Parent', ax, 'Position', [0.012 0 0.013 1], 'FaceColor', [0.9290 0.6940 0.1250 0.3], 'LineStyle', 'none');
                    if max(TopNanMat5(:, i, j)) - 0.025 > 0
                        rectangle('Parent', ax, 'Position', [0.025 0 max(TopNanMat5(:, i, j))-0.025 1], 'FaceColor', [0.8500 0.3250 0.0980 0.3], 'LineStyle', 'none');
                    end
                    
                    f = fit(TopNanMat5(:, i, j), TheoPERNanMat5(:, i, j), 'smoothingspline');
                    plot(TopNanMat5(:, i, j), PERNanMat5(:, i, j),  'xb', 'MarkerSize', 8);
                    plot(TopNanMat5(:, i, j), TheoPERNanMat5(:, i, j),  'or', 'MarkerSize', 3, 'MarkerFaceColor','r');
                    plot(f, '-r');
                    legend('Simulation data', 'Theoretical data', 'Fitting curve');
                    
                    name = ['PER vs BER in Topology 5, link ' num2str(i) '-' num2str(j)];
                    title(name);
                    lab = ['PER (pck size is ' num2str(BitsInPck) ' bits (including FEC))'];
                    ylabel(lab);
                    xlabel('BER');
                    ylim([0 1]);
                    xlim([0 max(TopNanMat5(:, i, j))]);
                end
            end
        end
        
    case 6
        for i = 1:4
            for j = 1:4
                if isnan(sum(TopNanMat6(:, i, j)))
                    disp('NaN in data; the link may not exist');
                else
                    fig = figure();
                    ax = axes('Parent', fig);
                    hold (ax, 'on');
                    
                    rectangle('Parent', ax, 'Position', [0 0 0.012 1], 'FaceColor', [0 1 0 0.3], 'LineStyle', 'none');
                    rectangle('Parent', ax, 'Position', [0.012 0 0.013 1], 'FaceColor', [0.9290 0.6940 0.1250 0.3], 'LineStyle', 'none');
                    if max(TopNanMat6(:, i, j)) - 0.025 > 0
                        rectangle('Parent', ax, 'Position', [0.025 0 max(TopNanMat6(:, i, j))-0.025 1], 'FaceColor', [0.8500 0.3250 0.0980 0.3], 'LineStyle', 'none');
                    end
                    
                    f = fit(TopNanMat6(:, i, j), TheoPERNanMat6(:, i, j), 'smoothingspline');
                    plot(TopNanMat6(:, i, j), PERNanMat6(:, i, j),  'xb', 'MarkerSize', 8);
                    plot(TopNanMat6(:, i, j), TheoPERNanMat6(:, i, j),  'or', 'MarkerSize', 3, 'MarkerFaceColor','r');
                    plot(f, '-r');
                    legend('Simulation data', 'Theoretical data', 'Fitting curve');
                    
                    name = ['PER vs BER in Topology 6, link ' num2str(i) '-' num2str(j)];
                    title(name);
                    lab = ['PER (pck size is ' num2str(BitsInPck) ' bits (including FEC))'];
                    ylabel(lab);
                    xlabel('BER');
                    ylim([0 1]);
                    xlim([0 max(TopNanMat6(:, i, j))]);
                end
            end
        end
    otherwise
        disp('Choose a valid Topology number');
end

%% Average PER matrices for the links

%Topology 1
AvgPER1 = zeros(4, 4); %tx node is in pos i, rx in pos j
for i = 1:4
    for j = 1:4
        AvgPER1(i, j) = sum(PERNanMat1(:, i, j))/length(PERNanMat1);
    end
end

%Topology 2
AvgPER2 = zeros(4, 4); %tx node is in pos i, rx in pos j
for i = 1:4
    for j = 1:4
        AvgPER2(i, j) = sum(PERNanMat2(:, i, j))/length(PERNanMat2);
    end
end

%Topology 3
AvgPER3 = zeros(4, 4); %tx node is in pos i, rx in pos j
for i = 1:4
    for j = 1:4
        AvgPER3(i, j) = sum(PERNanMat3(:, i, j))/length(PERNanMat3);
    end
end

%Topology 4
AvgPER4 = zeros(4, 4); %tx node is in pos i, rx in pos j
for i = 1:4
    for j = 1:4
        AvgPER4(i, j) = sum(PERNanMat4(:, i, j))/length(PERNanMat4);
    end
end

%Topology 5
AvgPER5 = zeros(4, 4); %tx node is in pos i, rx in pos j
for i = 1:4
    for j = 1:4
        AvgPER5(i, j) = sum(PERNanMat5(:, i, j))/length(PERNanMat5);
    end
end

%Topology 6
AvgPER6 = zeros(4, 4); %tx node is in pos i, rx in pos j
for i = 1:4
    for j = 1:4
        AvgPER6(i, j) = sum(PERNanMat6(:, i, j))/length(PERNanMat6);
    end
end
