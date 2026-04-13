using System;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.IO.Ports;
using System.Windows.Forms;
using ScottPlot;

namespace TR28386_T_PC
{
    public class MainForm : Form
    {
        private IpcProtocol _protocol;
        private System.Windows.Forms.Timer _timer;

        // Custom UI Colors - Dark Theme
        private Color colorBg = Color.FromArgb(30, 30, 30);
        private Color colorPanelBg = Color.FromArgb(45, 45, 48);
        private Color colorText = Color.FromArgb(220, 220, 220);
        private Color colorAccent = Color.FromArgb(0, 122, 204);
        private Color colorError = Color.FromArgb(200, 50, 50);

        private DateTime _lastRxTime = DateTime.MinValue;
        private bool _awaitingEepReadResponse = false;

        // Control Data Model
        private ControlMessageData _ctrlDto = new ControlMessageData();

        private ComboBox cmbPorts;
        private ComboBox cmbBauds;
        private Button btnConnect;
        private Button btnDisconnect;
        private Button btnInit;
        private Button btnRefresh;
        private Label lblPortConnected;
        private Label lblCommReceiving;

        // Status Arrays
        private Label[] lblLEDs; // 0=Tact01, 1=Tact02. To show ON/OFF
        private Label lblEncoderAngle;
        private Label lblEncoderRawPD;
        private Label lblPWMRaw;
        private Label lblPWMRCLPF;
        private Label lblPWMBWLPF;
        private Label lblPotenRAW;
        private Label lblPotenMAVE;
        private Label lblEepromReadValStatus;
        private Label lblIncNumber;

        // Controls
        private Button[] btnControlLEDs;
        private CheckBox chkEpwm7a;
        private TrackBar trbEpwmDuty;
        private Label lblEpwmDutyVal;
        private ComboBox cmbEpwmFreq;
        
        private TextBox txtEepAddr;
        private TextBox txtEepData;
        private Button btnEepWrite;
        private Button btnEepRead;

        // Graph Variables
        private const int GRAPH_MAX_POINTS = 2000;
        private double[] _bufPWMRaw = new double[GRAPH_MAX_POINTS];
        private double[] _bufPWMRCLPF = new double[GRAPH_MAX_POINTS];
        private double[] _bufPWMBWLPF = new double[GRAPH_MAX_POINTS];
        private double[] _bufPotenRaw = new double[GRAPH_MAX_POINTS];
        private double[] _bufPotenMave = new double[GRAPH_MAX_POINTS];
        private int _graphIndex = 0;
        private FormsPlot _formsPlot;
        private ScottPlot.Plottable.SignalPlot _sigRaw;
        private ScottPlot.Plottable.SignalPlot _sigRCLPF;
        private ScottPlot.Plottable.SignalPlot _sigBWLPF;
        private ScottPlot.Plottable.SignalPlot _sigPotenRaw;
        private ScottPlot.Plottable.SignalPlot _sigPotenMave;
        private bool _isGraphPaused = false;

        // Log
        private LogForm _logForm;
        private Label lblLogRxInfo;
        private Label lblLogTxInfo;

        public MainForm()
        {
            this.Text = "TR28386_T Monitoring & Dashboard";
            this.Size = new Size(2100, 1275);
            this.AutoScroll = true; // Allows scroll on small monitors
            this.StartPosition = FormStartPosition.CenterScreen;
            this.BackColor = colorBg;
            this.ForeColor = colorText;
            this.Font = new Font("맑은 고딕", 9F);

            _logForm = new LogForm();

            _protocol = new IpcProtocol();
            _protocol.OnStatusReceived += OnStatusReceived;
            _protocol.OnCommError += OnCommError;
            _protocol.OnPortClosed += () => Invoke((Action)UpdateConnectButtons);
            
            _protocol.OnRawTx += OnRawTxReceived;
            _protocol.OnRawRx += OnRawRxReceived;

            _timer = new System.Windows.Forms.Timer { Interval = 100 };
            _timer.Tick += Timer_Tick;

            BuildUI();
        }

        private void BuildUI()
        {
            TableLayoutPanel mainLayout = new TableLayoutPanel
            {
                Dock = DockStyle.Fill,
                RowCount = 4,
                ColumnCount = 3,
                Padding = new Padding(15)
            };
            mainLayout.RowStyles.Add(new RowStyle(SizeType.Absolute, 195)); // Comm row
            mainLayout.RowStyles.Add(new RowStyle(SizeType.Absolute, 360)); // Status row
            mainLayout.RowStyles.Add(new RowStyle(SizeType.Absolute, 390)); // Control row
            mainLayout.RowStyles.Add(new RowStyle(SizeType.Absolute, 160)); // Log row
            mainLayout.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 675));
            mainLayout.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 600));
            mainLayout.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100)); // Graph column

            // Top Bar - Communication Panel
            Panel pnlComm = CreateStyledPanel("COMMUNICATION");
            pnlComm.Dock = DockStyle.Fill;
            pnlComm.Margin = new Padding(5);
            mainLayout.SetColumnSpan(pnlComm, 2);

            // Labels
            Label lblPort = new Label { Text = "COM Port", Location = new Point(20, 50), AutoSize = true, Font = new Font("Segoe UI", 11, FontStyle.Bold) };
            cmbPorts = new ComboBox { 
                Location = new Point(160, 48), Width = 150,
                BackColor = Color.FromArgb(45, 45, 48), ForeColor = Color.FromArgb(0, 255, 200), 
                DropDownStyle = ComboBoxStyle.DropDownList, Font = new Font("맑은 고딕", 11, FontStyle.Regular)
            };
            UpdatePortsList();
            cmbPorts.Click += (s, e) => UpdatePortsList();

            btnRefresh = CreateBorderedButton("새로고침", 320, 45, 135, 50);
            btnRefresh.Click += (s, e) => UpdatePortsList();

            lblPortConnected = new Label { Text = "● 포트 연결됨", Location = new Point(480, 50), AutoSize = true, ForeColor = Color.Gray, Font = new Font("맑은 고딕", 11, FontStyle.Bold) };
            lblCommReceiving = new Label { Text = "● 통신 수신중", Location = new Point(720, 50), AutoSize = true, ForeColor = Color.Gray, Font = new Font("맑은 고딕", 11, FontStyle.Bold) };

            Label lblBaud = new Label { Text = "Baud Rate", Location = new Point(20, 120), AutoSize = true, Font = new Font("Segoe UI", 11, FontStyle.Bold) };
            cmbBauds = new ComboBox { 
                Location = new Point(160, 118), Width = 150,
                BackColor = Color.FromArgb(45, 45, 48), ForeColor = Color.FromArgb(0, 255, 200), 
                DropDownStyle = ComboBoxStyle.DropDownList, Font = new Font("맑은 고딕", 11, FontStyle.Regular)
            };
            cmbBauds.Items.AddRange(new object[] { "9600", "19200", "38400", "57600", "115200", "230400", "460800" });
            cmbBauds.SelectedItem = "115200";

            btnConnect = CreateBorderedButton("연결", 320, 115, 100, 50);
            btnConnect.Click += (s, e) => Connect();

            btnDisconnect = CreateBorderedButton("해제", 440, 115, 100, 50);
            btnDisconnect.Click += (s, e) => _protocol.Disconnect();

            btnInit = CreateBorderedButton("초기화", 560, 115, 100, 50);
            btnInit.Click += (s, e) => _protocol.ReInit();

            pnlComm.Controls.AddRange(new Control[] { 
                lblPort, lblBaud, cmbPorts, cmbBauds, btnRefresh, 
                btnConnect, btnDisconnect, btnInit,
                lblPortConnected, lblCommReceiving
            });

            pnlComm.Height = 190;
            mainLayout.Controls.Add(pnlComm, 0, 0);

            // Status Panel
            Panel pnlStatus = CreateStyledPanel("MCU Status Monitor (Status Message)");
            pnlStatus.Dock = DockStyle.Fill;
            mainLayout.SetColumnSpan(pnlStatus, 2);

            lblLEDs = new Label[2];
            lblLEDs[0] = CreateLedStatus("Tact01", 30, 35, pnlStatus);
            lblLEDs[1] = CreateLedStatus("Tact02", 225, 35, pnlStatus);

            lblEncoderAngle = CreateLabelValue("EncoderAngle:", "", 30, 95, pnlStatus);
            lblEncoderRawPD = CreateLabelValue("EncoderRawPD:", "", 700, 95, pnlStatus);
            
            lblPWMRaw = CreateLabelValue("PWMRaw:", "", 30, 150, pnlStatus);
            lblPWMRCLPF = CreateLabelValue("PWMRCLPF:", "", 700, 150, pnlStatus);
            
            lblPWMBWLPF = CreateLabelValue("PWMBWLPF:", "", 30, 205, pnlStatus);
            lblPotenRAW = CreateLabelValue("PotenRAW:", "", 700, 205, pnlStatus);
            
            lblPotenMAVE = CreateLabelValue("PotenMAVE:", "", 30, 260, pnlStatus);
            lblEepromReadValStatus = CreateLabelValue("EepromReadVal (Hex):", "", 700, 260, pnlStatus);
            
            lblIncNumber = CreateLabelValue("Seq (IncNumber):", "", 30, 315, pnlStatus);
            
            pnlStatus.Height = 270;

            // Control LED panel
            Panel pnlCtrls = CreateStyledPanel("MCU Controls (Control Message)");
            pnlCtrls.Dock = DockStyle.Fill;
            int cy = 45;
            btnControlLEDs = new Button[8];
            for (int i = 0; i < 8; i++)
            {
                int index = i;
                btnControlLEDs[index] = CreateBorderedButton($"LED{index + 1:00}", 30 + (index % 4) * 125, cy + (index / 4) * 65, 110, 50);
                btnControlLEDs[index].BackColor = Color.FromArgb(60, 60, 60);
                btnControlLEDs[index].Click += (s, e) => ToggleLed(index);
                pnlCtrls.Controls.Add(btnControlLEDs[index]);
            }

            cy = 195;
            chkEpwm7a = new CheckBox { Text = "Epwm7a Target Enable", Location = new Point(30, cy), AutoSize = true, ForeColor = Color.FromArgb(0, 190, 255), Font = new Font("맑은 고딕", 11, FontStyle.Bold) };
            chkEpwm7a.CheckedChanged += (s, e) => {
                _ctrlDto.Epwm7aEn = chkEpwm7a.Checked; 
                if(_ctrlDto.Epwm7aEn) SendControlMessage(); 
            };
            pnlCtrls.Controls.Add(chkEpwm7a);

            cy = 255;
            Label lblDuty = new Label { Text = "Duty(1~100):", Location = new Point(30, cy + 5), AutoSize = true };
            trbEpwmDuty = new TrackBar { Minimum = 1, Maximum = 100, Value = 50, Location = new Point(180, cy), Width = 225, TickStyle = TickStyle.None };
            lblEpwmDutyVal = new Label { Text = "50", Location = new Point(420, cy + 5), Width = 50, Font = new Font("Consolas", 12, FontStyle.Bold) };
            trbEpwmDuty.Scroll += (s, e) => { 
                lblEpwmDutyVal.Text = trbEpwmDuty.Value.ToString(); 
                _ctrlDto.Epwm7aDuty = (byte)trbEpwmDuty.Value; 
                if(_ctrlDto.Epwm7aEn) SendControlMessage();
            };
            _ctrlDto.Epwm7aDuty = (byte)trbEpwmDuty.Value;
            pnlCtrls.Controls.Add(lblDuty);
            pnlCtrls.Controls.Add(trbEpwmDuty);
            pnlCtrls.Controls.Add(lblEpwmDutyVal);

            cy = 320;
            Label lblFreq = new Label { Text = "Freq:", Location = new Point(30, cy + 5), AutoSize = true };
            cmbEpwmFreq = new ComboBox { 
                Location = new Point(180, cy), Width = 225, 
                BackColor = Color.FromArgb(45, 45, 48), ForeColor = Color.FromArgb(0, 255, 200), 
                DropDownStyle = ComboBoxStyle.DropDownList, 
                Font = new Font("맑은 고딕", 10)
            };
            cmbEpwmFreq.Items.AddRange(new object[] { "10Hz (0)", "100Hz (1)", "1kHz (2)", "10kHz (3)", "100kHz (4)", "1MHz (5)", "10MHz (6)" });
            cmbEpwmFreq.SelectedIndex = 2; // default 1kHz
            cmbEpwmFreq.SelectedIndexChanged += (s, e) => {
                _ctrlDto.Epwm7aFreq = (byte)cmbEpwmFreq.SelectedIndex;
                if(_ctrlDto.Epwm7aEn) SendControlMessage();
            };
            _ctrlDto.Epwm7aFreq = (byte)cmbEpwmFreq.SelectedIndex;
            pnlCtrls.Controls.Add(lblFreq);
            pnlCtrls.Controls.Add(cmbEpwmFreq);
            
            pnlCtrls.Height = 350;


            // EEPROM Control (Row 2, Col 1)
            Panel pnlEep = CreateStyledPanel("EEPROM Read/Write");
            pnlEep.Dock = DockStyle.Fill;
            int ey = 90;

            pnlEep.Controls.Add(new Label { Text = "EepAddr (Hex):", Location = new Point(35, ey), AutoSize = true });
            txtEepAddr = new TextBox { Location = new Point(210, ey-2), Width=135, BackColor = Color.FromArgb(60,60,60), ForeColor=Color.White, Text="0000", BorderStyle = BorderStyle.FixedSingle, Font=new Font("Consolas", 11) };
            pnlEep.Controls.Add(txtEepAddr);

            pnlEep.Controls.Add(new Label { Text = "EepData (Hex):", Location = new Point(35, ey+75), AutoSize = true });
            txtEepData = new TextBox { Location = new Point(210, ey+73), Width=135, BackColor = Color.FromArgb(60, 60, 60), ForeColor = Color.White, Text="00", BorderStyle = BorderStyle.FixedSingle, Font=new Font("Consolas", 11) };
            pnlEep.Controls.Add(txtEepData);

            btnEepWrite = CreateBorderedButton("Write", 375, ey-3, 135, 50);
            btnEepWrite.Click += (s, e) => DoEepWrite();
            pnlEep.Controls.Add(btnEepWrite);

            btnEepRead = CreateBorderedButton("Read", 375, ey+70, 135, 50);
            btnEepRead.Click += (s, e) => DoEepRead();
            pnlEep.Controls.Add(btnEepRead);
            
            pnlEep.Height = 350;


            // Add to MainLayout (pnlComm already added above)
            mainLayout.SetColumnSpan(pnlComm, 2);
            mainLayout.Controls.Add(pnlStatus, 0, 1);
            mainLayout.Controls.Add(pnlCtrls, 0, 2);
            mainLayout.Controls.Add(pnlEep, 1, 2);

            // Set up Real-Time Log Panel (Row 3, Spans 3 columns)
            Panel pnlLog = CreateStyledPanel("REAL-TIME LOG MONITOR");
            pnlLog.Dock = DockStyle.Fill;
            pnlLog.Margin = new Padding(5);

            lblLogRxInfo = new Label { Location = new Point(30, 55), AutoSize = true, Font = new Font("Consolas", 11, FontStyle.Bold), ForeColor = Color.Lime };
            lblLogTxInfo = new Label { Location = new Point(30, 115), AutoSize = true, Font = new Font("Consolas", 11, FontStyle.Bold), ForeColor = Color.Yellow };
            
            Button btnLogDetail = CreateBorderedButton("로그 상세 보기\n(6K History)", 0, 45, 300, 90);
            btnLogDetail.Anchor = AnchorStyles.Right | AnchorStyles.Top;
            btnLogDetail.Click += (s, e) => {
                if (_logForm.IsDisposed) _logForm = new LogForm();
                _logForm.Show();
                _logForm.BringToFront();
            };
            
            pnlLog.Controls.Add(lblLogRxInfo);
            pnlLog.Controls.Add(lblLogTxInfo);
            pnlLog.Controls.Add(btnLogDetail);
            
            // Assign dynamic location for button in Resize
            pnlLog.Resize += (s,e) => { btnLogDetail.Location = new Point(pnlLog.Width - 330, 45); };

            mainLayout.Controls.Add(pnlLog, 0, 3);
            mainLayout.SetColumnSpan(pnlLog, 3);

            // Set up Graph Panel (RowSpan = 3, Column = 2)
            Panel pnlGraph = CreateStyledPanel("Real-Time PWM Logs (20 sec)");
            pnlGraph.Dock = DockStyle.Fill;
            pnlGraph.Margin = new Padding(5);

            _formsPlot = new FormsPlot { Location = new Point(15, 45), Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right | AnchorStyles.Bottom };
            _formsPlot.Size = new Size(pnlGraph.Width - 30, pnlGraph.Height - 125);
            // Height calculates automatically based on anchors, setting initial size prevents zero-clip
            pnlGraph.Controls.Add(_formsPlot);
            
            pnlGraph.Resize += (s,e) => {
                _formsPlot.Height = pnlGraph.Height - 125; 
            };

            Button btnToggleGraph = CreateBorderedButton("Pause", 30, pnlGraph.Height - 65, 120, 50);
            btnToggleGraph.Anchor = AnchorStyles.Bottom | AnchorStyles.Left;
            btnToggleGraph.Click += (s, e) => {
                _isGraphPaused = !_isGraphPaused;
                btnToggleGraph.Text = _isGraphPaused ? "Resume" : "Pause";
                btnToggleGraph.BackColor = _isGraphPaused ? Color.Orange : Color.FromArgb(45, 45, 48);
            };

            Button btnClearGraph = CreateBorderedButton("Clear", 170, pnlGraph.Height - 65, 120, 50);
            btnClearGraph.Anchor = AnchorStyles.Bottom | AnchorStyles.Left;
            btnClearGraph.Click += (s, e) => {
                Array.Clear(_bufPWMRaw, 0, GRAPH_MAX_POINTS);
                Array.Clear(_bufPWMRCLPF, 0, GRAPH_MAX_POINTS);
                Array.Clear(_bufPWMBWLPF, 0, GRAPH_MAX_POINTS);
                Array.Clear(_bufPotenRaw, 0, GRAPH_MAX_POINTS);
                Array.Clear(_bufPotenMave, 0, GRAPH_MAX_POINTS);
                _graphIndex = 0;
                _formsPlot.Plot.SetAxisLimits(xMin: 0, xMax: GRAPH_MAX_POINTS);
                _formsPlot.Refresh();
            };

            pnlGraph.Controls.Add(btnToggleGraph);
            pnlGraph.Controls.Add(btnClearGraph);

            mainLayout.Controls.Add(pnlGraph, 2, 0);
            mainLayout.SetRowSpan(pnlGraph, 3);

            this.Controls.Add(mainLayout);
            
            InitGraph();
            UpdateConnectButtons();
        }

        private void InitGraph()
        {
            _formsPlot.Plot.Style(Style.Black);
            
            _sigRaw = _formsPlot.Plot.AddSignal(_bufPWMRaw, sampleRate: 1, color: Color.Cyan, label: "PWMRaw");
            _sigRCLPF = _formsPlot.Plot.AddSignal(_bufPWMRCLPF, sampleRate: 1, color: Color.Magenta, label: "PWMRCLPF");
            _sigBWLPF = _formsPlot.Plot.AddSignal(_bufPWMBWLPF, sampleRate: 1, color: Color.Lime, label: "PWMBWLPF");
            _sigPotenRaw = _formsPlot.Plot.AddSignal(_bufPotenRaw, sampleRate: 1, color: Color.Yellow, label: "PotenRAW");
            _sigPotenMave = _formsPlot.Plot.AddSignal(_bufPotenMave, sampleRate: 1, color: Color.OrangeRed, label: "PotenMAVE");
            
            _sigRaw.MaxRenderIndex = 0;
            _sigRCLPF.MaxRenderIndex = 0;
            _sigBWLPF.MaxRenderIndex = 0;
            _sigPotenRaw.MaxRenderIndex = 0;
            _sigPotenMave.MaxRenderIndex = 0;

            _formsPlot.Plot.SetAxisLimits(xMin: 0, xMax: GRAPH_MAX_POINTS, yMin: -0.1, yMax: 3.6);
            _formsPlot.Plot.YLabel("Value");
            _formsPlot.Plot.XLabel("Time (points)");
            _formsPlot.Plot.Legend(true, Alignment.UpperRight);
            _formsPlot.Refresh();
        }

        private void UpdatePortsList()
        {
            var ports = SerialPort.GetPortNames();
            string selected = cmbPorts.SelectedItem as string;
            cmbPorts.Items.Clear();
            cmbPorts.Items.AddRange(ports);
            if (ports.Length > 0)
            {
                if (selected != null && cmbPorts.Items.Contains(selected))
                    cmbPorts.SelectedItem = selected;
                else
                    cmbPorts.SelectedIndex = 0;
            }
        }

        private void Connect()
        {
            if (cmbPorts.SelectedItem == null || cmbBauds.SelectedItem == null) return;
            try
            {
                _protocol.Connect(cmbPorts.SelectedItem.ToString(), int.Parse(cmbBauds.SelectedItem.ToString()));
                UpdateConnectButtons();
                _timer.Start();

                // Send initial state
                SendControlMessage();
            }
            catch (Exception ex)
            {
                MessageBox.Show("Connection Failed: " + ex.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void UpdateConnectButtons()
        {
            bool isConn = _protocol.IsConnected;
            btnConnect.Enabled = !isConn;
            btnDisconnect.Enabled = isConn;
            btnInit.Enabled = isConn;

            if (isConn)
            {
                lblPortConnected.ForeColor = Color.Lime; // Bright green
            }
            else
            {
                lblPortConnected.ForeColor = Color.Gray;
                lblCommReceiving.ForeColor = Color.Gray;
                _timer.Stop();
            }
        }

        private void DoEepWrite()
        {
            ushort addr = ParseInput(txtEepAddr.Text);
            byte dataVal = (byte)ParseInput(txtEepData.Text);

            _ctrlDto.EepAddr = addr;
            _ctrlDto.EepromWriteVal = dataVal;
            _ctrlDto.EepWrite = true;
            _ctrlDto.EepRead = false;
            SendControlMessage();
            
            System.Threading.Tasks.Task.Run(async () => {
                await System.Threading.Tasks.Task.Delay(50);
                _ctrlDto.EepWrite = false;
            });
        }

        private void DoEepRead()
        {
            ushort addr = ParseInput(txtEepAddr.Text);
            
            _ctrlDto.EepAddr = addr;
            _ctrlDto.EepWrite = false;
            _ctrlDto.EepRead = true;
            SendControlMessage();

            System.Threading.Tasks.Task.Run(async () => {
                await System.Threading.Tasks.Task.Delay(100);
                _ctrlDto.EepRead = false;
                _awaitingEepReadResponse = true; // Signal UI to grab the fresh value
            });
        }

        private ushort ParseInput(string input)
        {
            if (string.IsNullOrWhiteSpace(input)) return 0;
            try
            {
                // Force hexadecimal parse
                string hexStr = input.Replace("0x", "").Replace("0X", "").Trim();
                return Convert.ToUInt16(hexStr, 16);
            }
            catch { return 0; }
        }

        private void ToggleLed(int index)
        {
            _ctrlDto.LEDs[index] = !_ctrlDto.LEDs[index];
            btnControlLEDs[index].BackColor = _ctrlDto.LEDs[index] ? Color.MediumSpringGreen : Color.Gray;
            btnControlLEDs[index].ForeColor = _ctrlDto.LEDs[index] ? Color.Black : Color.White;
            SendControlMessage();
        }

        private void SendControlMessage()
        {
            _protocol.SendControlMessage(_ctrlDto);
        }

        private void OnStatusReceived(StatusMessageData data)
        {
            BeginInvoke((Action)(() =>
            {
                _lastRxTime = DateTime.Now;
                lblCommReceiving.ForeColor = Color.Lime; 

                // Update UI from MCU Status
                UpdateLedStatus(lblLEDs[0], data.Tact01);
                UpdateLedStatus(lblLEDs[1], data.Tact02);

                lblEncoderAngle.Text = string.Format("{0:0.00}", data.EncoderAngle);
                lblEncoderRawPD.Text = data.EncoderRawPD.ToString();
                
                lblPWMRaw.Text = string.Format("{0:0.000}", data.PWMRaw);
                lblPWMRCLPF.Text = string.Format("{0:0.000}", data.PWMRCLPF);
                lblPWMBWLPF.Text = string.Format("{0:0.000}", data.PWMBWLPF);
                
                lblPotenRAW.Text = string.Format("{0:0.000}", data.PotenRAW);
                lblPotenMAVE.Text = string.Format("{0:0.000}", data.PotenMAVE);
                
                lblEepromReadValStatus.Text = data.EepromReadVal.ToString("X2");
                lblIncNumber.Text = data.IncNumber.ToString();

                UpdateGraphData(data.PWMRaw, data.PWMRCLPF, data.PWMBWLPF, data.PotenRAW, data.PotenMAVE);

                // if reading EEPROM requested and responded
                if (_awaitingEepReadResponse)
                {
                   txtEepData.Text = data.EepromReadVal.ToString("X2");
                   _awaitingEepReadResponse = false;
                }
            }));
        }

        private void OnCommError(string msg)
        {
            // UI Error indication ignored to keep interface clean
        }

        private void Timer_Tick(object sender, EventArgs e)
        {
            // Clear receiving indication if no data arrived in last 500ms
            if ((DateTime.Now - _lastRxTime).TotalMilliseconds > 500)
            {
                lblCommReceiving.ForeColor = Color.Gray;
            }

            if (!_isGraphPaused && _graphIndex > 0)
            {
                _formsPlot.Plot.SetAxisLimits(xMin: 0, xMax: GRAPH_MAX_POINTS, yMin: -0.1, yMax: 3.6);
                _formsPlot.Refresh();
            }
        }

        private void UpdateGraphData(double raw, double rclpf, double bwlpf, double pRaw, double pMave)
        {
            if (_isGraphPaused) return;

            if (_graphIndex < GRAPH_MAX_POINTS)
            {
                _bufPWMRaw[_graphIndex] = raw;
                _bufPWMRCLPF[_graphIndex] = rclpf;
                _bufPWMBWLPF[_graphIndex] = bwlpf;
                _bufPotenRaw[_graphIndex] = pRaw;
                _bufPotenMave[_graphIndex] = pMave;
                _graphIndex++;
            }
            else
            {
                Array.Copy(_bufPWMRaw, 1, _bufPWMRaw, 0, GRAPH_MAX_POINTS - 1);
                Array.Copy(_bufPWMRCLPF, 1, _bufPWMRCLPF, 0, GRAPH_MAX_POINTS - 1);
                Array.Copy(_bufPWMBWLPF, 1, _bufPWMBWLPF, 0, GRAPH_MAX_POINTS - 1);
                Array.Copy(_bufPotenRaw, 1, _bufPotenRaw, 0, GRAPH_MAX_POINTS - 1);
                Array.Copy(_bufPotenMave, 1, _bufPotenMave, 0, GRAPH_MAX_POINTS - 1);
                _bufPWMRaw[GRAPH_MAX_POINTS - 1] = raw;
                _bufPWMRCLPF[GRAPH_MAX_POINTS - 1] = rclpf;
                _bufPWMBWLPF[GRAPH_MAX_POINTS - 1] = bwlpf;
                _bufPotenRaw[GRAPH_MAX_POINTS - 1] = pRaw;
                _bufPotenMave[GRAPH_MAX_POINTS - 1] = pMave;
            }

            _sigRaw.MaxRenderIndex = _graphIndex - 1;
            _sigRCLPF.MaxRenderIndex = _graphIndex - 1;
            _sigBWLPF.MaxRenderIndex = _graphIndex - 1;
            _sigPotenRaw.MaxRenderIndex = _graphIndex - 1;
            _sigPotenMave.MaxRenderIndex = _graphIndex - 1;
        }

        protected override void OnFormClosing(FormClosingEventArgs e)
        {
            _protocol.Disconnect();
            base.OnFormClosing(e);
        }

        // --- Helper Methods to Draw Nice UI Elements ---
        private Panel CreateStyledPanel(string title)
        {
            Panel pnl = new Panel { BackColor = colorPanelBg, Padding = new Padding(5) };
            pnl.Paint += (s, e) => {
                e.Graphics.SmoothingMode = SmoothingMode.AntiAlias;
                Color mint = Color.FromArgb(0, 255, 200);
                Pen pen = new Pen(Color.FromArgb(80, 80, 80), 1);
                
                // Draw the border rectangle
                e.Graphics.DrawRectangle(pen, 5, 12, pnl.Width - 11, pnl.Height - 18);
                
                // Draw title background
                Font titleFont = new Font("맑은 고딕", 10, FontStyle.Bold);
                SizeF titleSize = e.Graphics.MeasureString(title, titleFont);
                e.Graphics.FillRectangle(new SolidBrush(colorPanelBg), 15, 2, titleSize.Width + 10, 20);
                
                // Draw Title
                e.Graphics.DrawString(title, titleFont, new SolidBrush(mint), new PointF(20, 2));
            };
            return pnl;
        }

        private Button CreateStyledButton(string text, int x, int y, int w)
        {
            return CreateBorderedButton(text, x, y, w, 30);
        }

        private Button CreateBorderedButton(string text, int x, int y, int w, int h)
        {
            Button btn = new Button {
                Text = text,
                Location = new Point(x, y),
                Width = w,
                Height = h,
                FlatStyle = FlatStyle.Flat,
                BackColor = Color.FromArgb(45, 45, 48),
                ForeColor = Color.White,
                Font = new Font("맑은 고딕", 10, FontStyle.Bold),
                Cursor = Cursors.Hand
            };
            btn.FlatAppearance.BorderColor = Color.FromArgb(0, 255, 200);
            btn.FlatAppearance.BorderSize = 1;
            return btn;
        }

        private Label CreateLedStatus(string title, int x, int y, Control parent)
        {
            Label lbl = new Label { Location = new Point(x, y + 2), AutoSize = true, Text = title, ForeColor = ControlPaint.Light(colorText), Font = new Font("Segoe UI", 10) };
            Label led = new Label { Location = new Point(x + 100, y), Width = 90, Height = 35, BackColor = Color.Gray };
            parent.Controls.Add(lbl);
            parent.Controls.Add(led);
            return led;
        }

        private void UpdateLedStatus(Label led, bool state)
        {
            if (state)
            {
                led.BackColor = Color.MediumSpringGreen;
                led.Text = "ON";
                led.ForeColor = Color.Black;
                led.TextAlign = ContentAlignment.MiddleCenter;
            }
            else
            {
                led.BackColor = Color.Gray;
                led.Text = "OFF";
                led.ForeColor = Color.White;
                led.TextAlign = ContentAlignment.MiddleCenter;
            }
        }

        private void OnRawRxReceived(byte[] rawData)
        {
            string hexData = BitConverter.ToString(rawData).Replace("-", " ");
            string timeStr = DateTime.Now.ToString("HH:mm:ss.fff");
            string line = $"[{timeStr}] RX: {hexData}";
            
            BeginInvoke((Action)(() =>
            {
                lblLogRxInfo.Text = line;
            }));
            
            if (_logForm != null) _logForm.AddLog(line);
        }

        private void OnRawTxReceived(byte[] rawData)
        {
            string hexData = BitConverter.ToString(rawData).Replace("-", " ");
            string timeStr = DateTime.Now.ToString("HH:mm:ss.fff");
            string line = $"[{timeStr}] TX: {hexData}";
            
            BeginInvoke((Action)(() =>
            {
                lblLogTxInfo.Text = line;
            }));
            
            if (_logForm != null) _logForm.AddLog(line);
        }

        private Label CreateLabelValue(string title, string defValue, int x, int y, Control parent)
        {
            Label lbl = new Label { Location = new Point(x, y), AutoSize = true, Text = title, ForeColor=ControlPaint.Light(colorText), Font=new Font("Segoe UI", 10) };
            Label val = new Label { Location = new Point(x + 240, y - 2), AutoSize = false, Size = new Size(200, 35), Text = defValue, Font=new Font("Consolas", 12, FontStyle.Bold), ForeColor=Color.White, TextAlign=ContentAlignment.MiddleLeft };
            parent.Controls.Add(lbl);
            parent.Controls.Add(val);
            return val;
        }
    }
}
