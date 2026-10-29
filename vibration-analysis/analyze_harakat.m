% MPU6050 tajriba CSV faylini MATLAB'da ochish va tahlil qilish.
% MATLAB R2020b yoki yangiroq. Qo'shimcha toolbox talab qilinmaydi.
% Sensor vaqti ishlatiladi; browser_batch_utc o'lchov vaqti emas.
[file, folder] = uigetfile('*.csv', 'Harakat monitoringi CSV faylini tanlang');
if isequal(file, 0), return; end
T = readtable(fullfile(folder, file), 'VariableNamingRule', 'preserve');
required = {'gateway_session','sensor_session','sensor_time_us', ...
    'sensor_valid','calibration_id','ax_g','ay_g','az_g', ...
    'gx_dps','gy_dps','gz_dps','accel_norm_g','relative_tilt_deg'};
assert(all(ismember(required, T.Properties.VariableNames)), ...
    'Bu fayl yangi Harakat monitoringi CSV formatida emas.');
T = T(T.sensor_valid == 1 & T.calibration_id > 0, :);
assert(~isempty(T), 'Kalibrlangan, yaroqli o''lchovlar topilmadi.');
% Turli qayta ishga tushishlar yoki tayanch holatlarni birlashtirmaymiz.
% Avtomatik ravishda eng oxirgi sessiya/kalibrlash tanlanadi.
last = T(end,:);
T = T(T.gateway_session == last.gateway_session & ...
      T.sensor_session == last.sensor_session & ...
      T.calibration_id == last.calibration_id, :);
[~, order] = sort(T.sensor_time_us); T = T(order,:);
[~, uniqueRows] = unique(T.sensor_time_us, 'stable'); T = T(uniqueRows,:);
assert(height(T) >= 10, 'Tahlil uchun kamida 10 ta yaroqli o''lchov kerak.');
t = (double(T.sensor_time_us) - double(T.sensor_time_us(1))) / 1e6;
dt = diff(t); Fs = 1 / median(dt);
fprintf('Tanlangan o''lchovlar: %d | Davomiylik: %.2f s | Median Fs: %.2f Hz\n', ...
    height(T), t(end), Fs);
fprintf('Eng katta vaqt oralig''i: %.4f s | Tayanch ID: %d\n', ...
    max(dt), last.calibration_id);
if any(dt > 3*median(dt))
    warning('Qaydda bo''shliqlar mavjud. Spektr uchun uzluksiz qism tanlanadi.');
end
figure('Name','Harakat va tebranish monitoringi','Color','w');
tiledlayout(3,1,'TileSpacing','compact');
nexttile; plot(t, [T.ax_g T.ay_g T.az_g]); grid on;
ylabel('Tezlanish, g'); legend('X','Y','Z'); title('Me''yorlangan akselerometr o''qlari');
nexttile; plot(t, T.relative_tilt_deg, 'LineWidth', 1.2); grid on;
ylabel('Qiyalik, daraja'); title('Joriy tayanch holatiga nisbatan qiyalik');
nexttile; plot(t, [T.gx_dps T.gy_dps T.gz_dps]); grid on;
xlabel('Sensor vaqti, s'); ylabel('Burchak tezligi, daraja/s');
legend('X','Y','Z'); title('Nol siljishi tuzatilgan giroskop');

% X o'qining taxminiy spektri: 50 ms dan katta bo'shliqlarni kesib,
% eng uzun qismni bir xil vaqt qadamiga interp1 bilan o'tkazamiz.
% Interpolatsiya yo'qolgan o'lchovlarni tiklamaydi; bu taxminiy spektr.
cuts = [0; find(dt > 0.05); height(T)];
[~, longest] = max(diff(cuts));
idx = (cuts(longest)+1):cuts(longest+1);
if numel(idx) >= 128 && t(idx(end))-t(idx(1)) >= 2
    ti = t(idx); si = T.ax_g(idx);
    uniformTime = (ti(1):1/Fs:ti(end))';
    signal = interp1(ti, si, uniformTime, 'linear');
    signal = signal - mean(signal);
    N = numel(signal); window = 0.5-0.5*cos(2*pi*(0:N-1)'/(N-1));
    spectrum = abs(fft(signal.*window))/sum(window);
    spectrum = spectrum(1:floor(N/2)+1);
    if rem(N,2)==0, spectrum(2:end-1)=2*spectrum(2:end-1);
    else, spectrum(2:end)=2*spectrum(2:end); end
    frequency = (0:floor(N/2))'*Fs/N;
    figure('Name','Taxminiy X-o''qi spektri','Color','w');
    plot(frequency, spectrum); grid on; xlim([0 Fs/2]);
    xlabel('Chastota, Hz'); ylabel('Amplituda, g');
    title('X o''qi: vaqt bo''yicha tekislangan qaydning taxminiy spektri');
else
    fprintf('Spektr chiqarilmadi: kamida 2 s va 128 o''lchovli uzluksiz qism kerak.\n');
end
