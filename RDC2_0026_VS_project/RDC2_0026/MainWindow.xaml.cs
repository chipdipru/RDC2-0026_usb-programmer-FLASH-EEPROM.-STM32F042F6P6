/*
********************************************************************************
* COPYRIGHT(c) ЗАО «ЧИП и ДИП», 2017-2018
* 
* Программное обеспечение предоставляется на условиях «как есть» (as is).
* При распространении указание автора обязательно.
********************************************************************************
*/


using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;

namespace RDC2_0026
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private const byte USB_REPORT_ID_POS = 0;
        private const byte USB_CMD_POS = 1;
        private const byte USB_DATA_SIZE_POS = 1;
        private const byte USB_START_DATA_POS = 2;

        private const byte USB_CMD_ID = 1;
        private const byte USB_DATA_ID = 2;

        private const byte USB_CMD_MEMORY_INIT = 0;
        private const byte USB_CMD_MEMORY_READ = 1;
        private const byte USB_CMD_MEMORY_WRITE = 2;
        private const byte USB_CMD_MEMORY_ERASE = 3;
        private const byte USB_CMD_GET_STATE = 4;
        private const byte USB_CMD_MEMORY_BLANK_CHECK = 5;
        private const byte USB_CMD_SPI_READ_STATUS = 6;
        private const byte USB_CMD_SPI_WRITE_STATUS = 7;

        private const byte USB_CMD_PACKET_SIZE = 64;
        
        private const byte MEMORY_TYPE_OFFSET = (USB_CMD_POS + 1);
        private const byte MEMORY_VOLUME_OFFSET = (MEMORY_TYPE_OFFSET + 1);
        private const byte MEMORY_PAGE_SIZE_OFFSET = (MEMORY_VOLUME_OFFSET + 4);
        private const byte MEMORY_ADDRESS_BYTES_OFFSET = (MEMORY_PAGE_SIZE_OFFSET + 2);
        private const byte MEMORY_ADDRESS_BITS_OFFSET = (MEMORY_ADDRESS_BYTES_OFFSET + 1);
        private const byte MEMORY_FREQUENCY_OFFSET = (MEMORY_ADDRESS_BITS_OFFSET + 1);
        private const byte MEMORY_SPI_CS_OFFSET = (MEMORY_FREQUENCY_OFFSET + 1);
        private const byte MEMORY_START_ADDRESS_OFFSET = (MEMORY_SPI_CS_OFFSET + 1);
        private const byte MEMORY_DEINIT_FLAG_OFFSET = (MEMORY_START_ADDRESS_OFFSET + 4);


        private static readonly string[] TypeStrings = { "I2C EEPROM / FRAM", "SPI EEPROM / FRAM", "SPI FLASH", "Microwire EEPROM", };
        private static readonly string[] SPI_FLASH_VolumeStrings = { "256 кбит", "512 кбит", "1 Мбит", "2 Мбит", "4 Мбит", "8 Мбит", "16 Мбит", "32 Мбит", "64 Мбит", "128 Мбит", };

        private static readonly int[] SPI_FLASH_VolumeInBytes = { 32768, 65536, 131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608, 16777216, };

        private static readonly string[] I2C_SPI_EEPROM_VolumeStrings = { "1 кбит", "2 кбит", "4 кбит", "8 кбит", "16 кбит", "32 кбит", "64 кбит", "128 кбит", "256 кбит", "512 кбит",
                                                     "1 Мбит", "2 Мбит", };

        private static readonly int[] I2C_SPI_EEPROM_VolumeInBytes = { 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, };

        private static readonly string[] MicrowireVolumeStrings = { "1 кбит (xx93xx46)", "2 кбит (xx93xx57)", "2 кбит (xx93xx56)", "4 кбит (xx93xx66)", "8 кбит (xx93xx76)", "16 кбит (xx93xx86)", };

        private static readonly int[] MicrowireVolumeInBytes = { 128, 256, 256, 512, 1024, 2048, };

        private static readonly int[] Microwire8BitOrgAddrBits = { 7, 8, 9, 9, 11, 11, };

        private static readonly string[] PageSizeStrings = { "8 байт", "16 байт", "32 байта", "64 байта", "128 байт", "256 байт", };

        private static readonly int[] PageSizeByte = { 8, 16, 32, 64, 128, 256, };

        private static readonly string[] OrganizationStrings = { "8 бит", "16 бит", };

        private static readonly string[] I2CFreqStrings = { "100 кГц", "400 кГц", "1 MГц", };

        private static readonly string[] SPIFreqStrings = { "0,75 MГц", "1,5 MГц", "3 MГц", "6 MГц", "12 MГц", "24 MГц", };

        private static readonly byte[] SPIFreqDevider = { 5, 4, 3, 2, 1, 0, };

        private static readonly string[] MicrowireFreqStrings = { "250 кГц", "500 кГц", "1 MГц", "2 MГц", };

        private static readonly byte[] MicrowireFreqDevider = { 96, 48, 24, 12, };

        private static readonly string[] I2CConnectStrings = { "soic-8 / dip-8", "разъем I2C", };

        private static readonly string[] I2CConnectImages = { "images/RDC2-0026_I2C_SOIC_DIP_PBS.png", "images/RDC2-0026_I2C_PLS.png", };
        
        private static readonly string[] SPIConnectStrings = { "soic-8 / dip-8", "soic-16", "разъем SPI", };

        private static readonly string[] SPIConnectImages = { "images/RDC2-0026_SPI_SOIC_DIP_PBS.png", "images/RDC2-0026_SPI_SOIC_16.png", "images/RDC2-0026_SPI_PLS.png", };

        private static readonly byte[] SPIChipSel = {0, 2, 3, };

        private static readonly string[] MicrowireConnectStrings = { "soic-8 / dip-8", "разъем SPI", };

        private static readonly string[] MicrowireConnectImages = { "images/RDC2-0026_MICROWIRE_SOIC.png", "images/RDC2-0026_MICROWIRE_PLS.png", };

        private static readonly byte[] MicrowireChipSel = { 1, 3, };


        private static readonly string ChipNameString = "Наименование = ";
        private static readonly string ManufactureString = "Производитель = ";
        private static readonly string TypeString = "Тип = ";
        private static readonly string VolumeString = "Объем = ";
        private static readonly string PageSizeString = "Размер страницы = ";
        private static readonly string OrganizationString = "Организация = ";
        private static readonly string FrequencyString = "Тактовая частота = ";
        private static readonly string ConnectionString = "Подключение = ";
        private static readonly string ProgrammerConnected = "Программатор RDC2-0026 подключен\n";
        private static readonly string ProgrammerNotConnected = "Программатор не подключен. Подключите программатор и перезапустите программу.\n";
        private static readonly string ConnectingToChip = "Подключение к микросхеме...\n";
        private static readonly string ChipDetected = "Микросхема памяти обнаружена\n";
        private static readonly string ChipNotDetected = "Микросхема памяти не обнаружена\n";
        private static readonly string Read_JEDEC_ID = "Чтение ID (команда 0x9F)...\n";
        private static readonly string JEDEC_ID_NotSupported = "Команда чтения ID (0x9F) не поддерживается\n";
        private static readonly string ManufactureID = "ID производителя: ";
        private static readonly string DeviceID = "ID микросхемы: ";
        private static readonly string OperationComplete = "Операция завершена. ";
        private static readonly string ExeTime = "Время выполнения: ";
        private static readonly string Reading = "Выполняется чтение...";
        private static readonly string Erasing = "Выполняется стирание...";
        private static readonly string Writing = "Выполняется запись...";
        private static readonly string VerifyAfterWrite = "Выполняется проверка...";
        private static readonly string FileComparing = "Сравнение файлов...";
        private static readonly string BlankChecking = "Выполняется проверка на чистоту...";
        private static readonly string StatusReading = "Выполняется чтение регистра состояния...\n";
        private static readonly string StatusWriting = "Выполняется запись регистра состояния...\n";
        private static readonly string FileNotChosen = "Не выбран файл\n";
        private static readonly string SameFileChosenTwice = "Для чтения и сравнения выбран один файл\n";
        private static readonly string FileIsSmall = "Размер выбранного файла меньше объема памяти\nОставшаяся часть памяти будет стерта (заполнена 0xFF)\n";
        private static readonly string FileIsBig = "Размер выбранного файла больше объема памяти\nФайл будет записан с начала до заполнения памяти\n";
        private static readonly string MemoryIsBlank = "Микросхема памяти чистая\n";
        private static readonly string MemoryIsNotBlank = "Микросхема памяти содержит данные, начальный адрес данных: ";
        private static readonly string CompareFileIsSmall = "Размер файла для сравнения меньше объема памяти\n";
        private static readonly string CompareFileIsBig = "Размер файла для сравнения больше объема памяти\n";
        private static readonly string CompareFileEqual = "Размер файла для сравнения равен объему памяти\n";
        private static readonly string FilesIdentical = "Содержимое памяти и файла одинаково\n";
        private static readonly string FilesDifferent = "Содержимое памяти и файла различно с адреса: ";

        private static readonly string BinaryFileFilter = "Binary files (*.bin)|*.bin";
        private static readonly string SigmaStudioFileFilter = "SigmaStudio files (*.hex)|*.hex";


        private const byte I2C_EEPROM_FRAM = 0;
        private const byte SPI_EEPROM_FRAM = 1;
        private const byte SPI_FLASH = 2;
        private const byte Microwire_EEPROM = 3;

        private const byte I2C_MEMORY = 1;
        private const byte SPI_MEMORY_EEPROM = 2;
        private const byte SPI_MEMORY_FLASH = 3;
        private const byte MICROWIRE_EEPROM_ORG_8 = 4;
        private const byte MICROWIRE_EEPROM_ORG_16 = 5;

        private const int PROGRAMMER_DATA_BUFFER = 4096;
        private const int ONE_DATA_PORTION = PROGRAMMER_DATA_BUFFER / 4;
        private const int MAX_DATA_SIZE_PER_PACKET = 62;

        private const byte MEMORY_PRESENT = 0xAA;
        private const byte MEMORY_BLANK = (1 << 0);
        private const byte PROG_USB_BUF_OVERFLOW = (1 << 5);
        private const byte PROG_USB_OPER_COMPLETE = (1 << 6);

        private const bool NOT_DEINIT_AFTER_CONNECTION = false;
        private const bool DEINIT_AFTER_CONNECTION = true;

        private const int MAX_INFO_STRING_COUNT = 30;        



        private int MemVolume;
        private Stopwatch TimeMeas = new Stopwatch();
        private DispatcherTimer ShowProgressTimer = new DispatcherTimer();



        public MainWindow()
        {
            InitializeComponent();
            
            DataContext = this;

            TypeСomboBox.ItemsSource = TypeStrings;
            VolumeСomboBox.ItemsSource = I2C_SPI_EEPROM_VolumeStrings;
            PageSizeСomboBox.ItemsSource = PageSizeStrings;
            OrgСomboBox.ItemsSource = OrganizationStrings;
            FreqСomboBox.ItemsSource = I2CFreqStrings;
            ConnectСomboBox.ItemsSource = I2CConnectStrings;
            ConnectionImage.Source = new BitmapImage(new Uri("images/RDC2-0026_I2C_SOIC_DIP_PBS.png", UriKind.Relative));

            string DefaultFilePath = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetEntryAssembly().Location) + "\\DefaultChip.txt";
            if (File.Exists(DefaultFilePath))
                ChooseChip(DefaultFilePath);

            Boolean USBDevDetected = USB_device.Open();
            
            if (USBDevDetected)
            {
                InfoBox.Text = ProgrammerConnected;
                ShowProgressTimer.Interval = TimeSpan.FromMilliseconds(1000);
                ShowProgressTimer.Tick += ShowProgressTimer_Tick;
            }

            else
                InfoBox.Text = ProgrammerNotConnected;

            InfoBox.Text += "\n";
        }

        private void TypeChanged(object sender, SelectionChangedEventArgs e)
        {
            string[] ImagesPaths = new string[3];
            StatusBorder.IsEnabled = false;

            if (TypeСomboBox.SelectedIndex != Microwire_EEPROM)
            {
                PageSizeСomboBox.IsEnabled = true;
                OrgСomboBox.IsEnabled = false;
                ConnectCheckbutton.IsEnabled = true;

                if (TypeСomboBox.SelectedIndex == I2C_EEPROM_FRAM)
                {
                    VolumeСomboBox.ItemsSource = I2C_SPI_EEPROM_VolumeStrings;
                    FreqСomboBox.ItemsSource = I2CFreqStrings;
                    ConnectСomboBox.ItemsSource = I2CConnectStrings;
                    ImagesPaths = I2CConnectImages;
                }

                else if ((TypeСomboBox.SelectedIndex == SPI_EEPROM_FRAM) || (TypeСomboBox.SelectedIndex == SPI_FLASH))
                {
                    StatusBorder.IsEnabled = true;
                    FreqСomboBox.ItemsSource = SPIFreqStrings;
                    ConnectСomboBox.ItemsSource = SPIConnectStrings;
                    ImagesPaths = SPIConnectImages;

                    if (TypeСomboBox.SelectedIndex == SPI_EEPROM_FRAM)
                        VolumeСomboBox.ItemsSource = I2C_SPI_EEPROM_VolumeStrings;
                    else
                        VolumeСomboBox.ItemsSource = SPI_FLASH_VolumeStrings;
                }
            }

            else
            {
                VolumeСomboBox.ItemsSource = MicrowireVolumeStrings;
                FreqСomboBox.ItemsSource = MicrowireFreqStrings;
                ConnectСomboBox.ItemsSource = MicrowireConnectStrings;
                ImagesPaths = MicrowireConnectImages;
                PageSizeСomboBox.IsEnabled = false;
                OrgСomboBox.IsEnabled = true;
                ConnectCheckbutton.IsEnabled = false;
            }

            VolumeСomboBox.SelectedIndex = 0;
            PageSizeСomboBox.SelectedIndex = 0;
            OrgСomboBox.SelectedIndex = 0;
            FreqСomboBox.SelectedIndex = 0;
            ConnectСomboBox.SelectedIndex = 0;

            ConnectionImage.Source = new BitmapImage(new Uri(ImagesPaths[ConnectСomboBox.SelectedIndex], UriKind.Relative));
        }

        private void ConnectСomboBoxSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (ConnectСomboBox.SelectedItem != null)
            {
                string[] ImagesPaths = new string[3];

                if (ConnectСomboBox.ItemsSource == I2CConnectStrings)
                    ImagesPaths = I2CConnectImages;
                else if (ConnectСomboBox.ItemsSource == SPIConnectStrings)
                    ImagesPaths = SPIConnectImages;
                else if (ConnectСomboBox.ItemsSource == MicrowireConnectStrings)
                    ImagesPaths = MicrowireConnectImages;

                ConnectionImage.Source = new BitmapImage(new Uri(ImagesPaths[ConnectСomboBox.SelectedIndex], UriKind.Relative));
            }
        }
        
        private void ChooseChipbuttonClick(object sender, RoutedEventArgs e)
        {
            OpenFileDialog ChooseFile = new OpenFileDialog();
            ChooseFile.Filter = "Text files (*.txt)|*.txt";
            if (ChooseFile.ShowDialog() == true)
            {
                ChooseChip(ChooseFile.FileName);
            }
        }
        
        private void SaveChipbuttonClick(object sender, RoutedEventArgs e)
        {
            SaveFileDialog ChooseFile = new SaveFileDialog();
            ChooseFile.Filter = "Text files (*.txt)|*.txt";
            if (ChooseFile.ShowDialog() == true)
            {
                SaveChip(ChooseFile.FileName);
            }
        }

        private void WriteButtonClick(object sender, RoutedEventArgs e)
        {
            if (FileToWriteFromTextBox.Text.Length == 0)
            {
                InfoBox.Text += FileNotChosen + "\n";
                InfoBoxManager();
                return;
            }

            if (!ConnectToChip(NOT_DEINIT_AFTER_CONNECTION, true))
            {
                InfoBoxManager();
                return;
            }

            string OperationSourceFile = FileToWriteFromTextBox.Text;

            if (SigmaStudioFileCheck.IsChecked == true)
            {
                OperationSourceFile = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetEntryAssembly().Location) + "\\SigmaStudioBINFile.bin";

                SigmaStudioFileConverter(FileToWriteFromTextBox.Text, OperationSourceFile);
            }

            long FileSize = new FileInfo(OperationSourceFile).Length;

            if (MemVolume > FileSize)
                InfoBox.Text += FileIsSmall;
            else if (MemVolume < FileSize)
                InfoBox.Text += FileIsBig;
            
            BackOperation MemoryOperation = new BackOperation { InitOperation = Writing, CurrentOperation = Writing, SourceFile = OperationSourceFile };

            InfoBox.Text += MemoryOperation.CurrentOperation;
            
            ChipBorder.IsEnabled = false;
            OperationBorder.IsEnabled = false;
            
            TimeMeas.Reset();

            BackgroundWorker WriteBackground = new BackgroundWorker();
            WriteBackground.WorkerReportsProgress = true;
            WriteBackground.ProgressChanged += Background_ProgressChanged;
            WriteBackground.DoWork += WriteBackground_DoWork;
            WriteBackground.RunWorkerCompleted += Background_RunWorkerCompleted;
            WriteBackground.RunWorkerAsync(MemoryOperation);
        }

        private void ReadbuttonClick(object sender, RoutedEventArgs e)
        {
            if ((FileToReadToTextBox.Text.Length == 0) ||
               ((CompareFilesCheck.IsChecked == true) && (FileToCompareToTextBox.Text.Length == 0)))
            {
                InfoBox.Text += FileNotChosen + "\n";
                InfoBoxManager();
                return;
            }

            if (!ConnectToChip(NOT_DEINIT_AFTER_CONNECTION, true))
            {
                InfoBoxManager();
                return;
            }

            if (!File.Exists(FileToReadToTextBox.Text))
            {
                BinaryWriter SourceFile = new BinaryWriter(File.Create(FileToReadToTextBox.Text));
                SourceFile.Close();
            }

            if ((CompareFilesCheck.IsChecked == true) && (FileToReadToTextBox.Text == FileToCompareToTextBox.Text))
            {
                InfoBox.Text += SameFileChosenTwice + "\n";
                InfoBoxManager();
                return;
            }

            string OperationCompareToFile = FileToCompareToTextBox.Text;

            if (SigmaStudioFileCheck.IsChecked == true)
            {
                OperationCompareToFile = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetEntryAssembly().Location) + "\\SigmaStudioBINFile.bin";

                SigmaStudioFileConverter(FileToWriteFromTextBox.Text, OperationCompareToFile);
            }

            BackOperation MemoryOperation = new BackOperation { InitOperation = Reading, CurrentOperation = Reading, SourceFile = FileToReadToTextBox.Text, CompareToFile = OperationCompareToFile };

            InfoBox.Text += MemoryOperation.CurrentOperation;

            ChipBorder.IsEnabled = false;
            OperationBorder.IsEnabled = false;

            TimeMeas.Reset();

            BackgroundWorker ReadBackground = new BackgroundWorker();
            ReadBackground.WorkerReportsProgress = true;
            ReadBackground.ProgressChanged += Background_ProgressChanged;
            ReadBackground.DoWork += ReadBackground_DoWork;
            ReadBackground.RunWorkerCompleted += Background_RunWorkerCompleted;
            ReadBackground.RunWorkerAsync(MemoryOperation);
        }
        
        private void WriteBackground_DoWork(object sender, DoWorkEventArgs e)
        {
            BackOperation Operation = e.Argument as BackOperation;
            BinaryReader SourceFile = new BinaryReader(File.Open(Operation.SourceFile, FileMode.Open));

            byte[] USBPacket = new byte[USB_CMD_PACKET_SIZE];
            int MemAddress = 0;
            int Progress;

            USBPacket[USB_REPORT_ID_POS] = USB_CMD_ID;
            USBPacket[USB_CMD_POS] = USB_CMD_MEMORY_WRITE;

            SourceFile.BaseStream.Position = 0;

            TimeMeas.Start();

            bool USBSuccess = USB_device.Write(USBPacket);

            long FileLength = SourceFile.BaseStream.Length;

            byte[] DataToWrite = new byte[MemVolume];

            for (int i = 0; i < MemVolume; i++)
            {
                if (SourceFile.BaseStream.Position < FileLength)
                    DataToWrite[i] = SourceFile.ReadByte();
                else
                    DataToWrite[i] = 0xFF;
            }

            if (MemVolume <= PROGRAMMER_DATA_BUFFER)
            {
                int FullSizePacketCount = MemVolume / MAX_DATA_SIZE_PER_PACKET;

                USBPacket[USB_REPORT_ID_POS] = USB_DATA_ID;
                USBPacket[USB_DATA_SIZE_POS] = MAX_DATA_SIZE_PER_PACKET;

                for (int i = 0; i < FullSizePacketCount; i++)
                {
                    for (int datapos = 0; datapos < MAX_DATA_SIZE_PER_PACKET; datapos++)
                        USBPacket[USB_START_DATA_POS + datapos] = DataToWrite[MemAddress + datapos];

                    USBSuccess = USB_device.Write(USBPacket);
                    MemAddress += MAX_DATA_SIZE_PER_PACKET;

                    Progress = ((MemAddress * 100) / MemVolume);
                    (sender as BackgroundWorker).ReportProgress(Progress, Operation.CurrentOperation);
                }

                int RestData = MemVolume - FullSizePacketCount * MAX_DATA_SIZE_PER_PACKET;
                USBPacket[USB_DATA_SIZE_POS] = (byte)RestData;
                for (int datapos = 0; datapos < RestData; datapos++)
                    USBPacket[USB_START_DATA_POS + datapos] = DataToWrite[MemAddress + datapos];
                USBSuccess = USB_device.Write(USBPacket);
            }

            else
            {
                bool NextKByteNeeded = true;
                int IncomingDataPos = 0;
                int OneKBytePacketCount = MemVolume / ONE_DATA_PORTION;

                for (int KBytePack = 0; KBytePack < OneKBytePacketCount; KBytePack++)
                {
                    USBPacket[USB_REPORT_ID_POS] = USB_CMD_ID;
                    USBPacket[USB_CMD_POS] = USB_CMD_GET_STATE;

                    while (!NextKByteNeeded)
                    {
                        int ProgrammerDataPos = 0;

                        USBSuccess = USB_device.Write(USBPacket);
                        USBSuccess = USB_device.Read(USBPacket);
                        byte ProgrammerStatus = USBPacket[USB_START_DATA_POS];
                        ProgrammerDataPos = (USBPacket[USB_START_DATA_POS + 2] << 8) | USBPacket[USB_START_DATA_POS + 1];

                        if (((ProgrammerStatus & PROG_USB_BUF_OVERFLOW) != PROG_USB_BUF_OVERFLOW)
                         || ((((ProgrammerStatus & PROG_USB_BUF_OVERFLOW) == PROG_USB_BUF_OVERFLOW)) && (ProgrammerDataPos > IncomingDataPos) && ((ProgrammerDataPos - IncomingDataPos) >= ONE_DATA_PORTION)))
                        {
                            NextKByteNeeded = true;
                        }
                    }

                    USBPacket[USB_REPORT_ID_POS] = USB_DATA_ID;
                    USBPacket[USB_DATA_SIZE_POS] = MAX_DATA_SIZE_PER_PACKET;

                    //передача 1 кбайта данных начало
                    for (int i = 0; i < 16; i++)
                    {
                        for (int datapos = 0; datapos < MAX_DATA_SIZE_PER_PACKET; datapos++)
                            USBPacket[USB_START_DATA_POS + datapos] = DataToWrite[MemAddress + datapos];

                        USBSuccess = USB_device.Write(USBPacket);
                        MemAddress += MAX_DATA_SIZE_PER_PACKET;

                    }

                    USBPacket[USB_DATA_SIZE_POS] = 32;
                    for (int datapos = 0; datapos < 32; datapos++)
                        USBPacket[USB_START_DATA_POS + datapos] = DataToWrite[MemAddress + datapos];

                    USBSuccess = USB_device.Write(USBPacket);
                    MemAddress += 32;
                    //передача 1 кбайта данных окончание

                    IncomingDataPos += ONE_DATA_PORTION;

                    if (IncomingDataPos == PROGRAMMER_DATA_BUFFER)
                        IncomingDataPos = 0;

                    NextKByteNeeded = false;

                    Progress = ((MemAddress * 100) / MemVolume);
                    (sender as BackgroundWorker).ReportProgress(Progress, Operation.CurrentOperation);
                }
            }

            byte ProgStatus = 0;
            while ((ProgStatus & PROG_USB_OPER_COMPLETE) != PROG_USB_OPER_COMPLETE)
            {
                USBPacket[USB_REPORT_ID_POS] = USB_CMD_ID;
                USBPacket[USB_CMD_POS] = USB_CMD_GET_STATE;
                USBSuccess = USB_device.Write(USBPacket);
                USBSuccess = USB_device.Read(USBPacket);
                ProgStatus = USBPacket[USB_START_DATA_POS];
            }

            TimeMeas.Stop();
            SourceFile.Close();
            e.Result = e.Argument;
        }

        private void ReadBackground_DoWork(object sender, DoWorkEventArgs e)
        {
            BackOperation Operation = e.Argument as BackOperation;
            BinaryWriter SourceFile = new BinaryWriter(File.Open(Operation.SourceFile, FileMode.Create));

            int Progress;
            byte[] USBPacket = new byte[USB_CMD_PACKET_SIZE];
            int MemAdr = 0;

            USBPacket[USB_REPORT_ID_POS] = USB_CMD_ID;
            USBPacket[USB_CMD_POS] = USB_CMD_MEMORY_READ;

            SourceFile.BaseStream.Position = 0;
            TimeMeas.Start();

            bool USBSuccess = USB_device.Write(USBPacket);

            while (MemAdr < MemVolume)
            {
                USBSuccess = USB_device.Read(USBPacket);

                SourceFile.Write(USBPacket);

                MemAdr += USB_CMD_PACKET_SIZE;

                Progress = ((MemAdr * 100) / MemVolume);
                (sender as BackgroundWorker).ReportProgress(Progress, Operation.CurrentOperation);
            }

            byte ProgStatus = 0;
            while ((ProgStatus & PROG_USB_OPER_COMPLETE) != PROG_USB_OPER_COMPLETE)
            {
                USBPacket[USB_REPORT_ID_POS] = USB_CMD_ID;
                USBPacket[USB_CMD_POS] = USB_CMD_GET_STATE;
                USBSuccess = USB_device.Write(USBPacket);
                USBSuccess = USB_device.Read(USBPacket);
                ProgStatus = USBPacket[USB_START_DATA_POS];
            }

            TimeMeas.Stop();
            SourceFile.Close();
            e.Result = e.Argument;
        }

        private void CompareFilesBackground_DoWork(object sender, DoWorkEventArgs e)
        {
            long FilePosition = 0;
            BackOperation Operation = e.Argument as BackOperation;

            BinaryReader SourceFile = new BinaryReader(File.Open(Operation.SourceFile, FileMode.Open));
            BinaryReader CompareToFile = new BinaryReader(File.Open(Operation.CompareToFile, FileMode.Open));

            long SourceFileSize = SourceFile.BaseStream.Length;
            long CompareToFileSize = CompareToFile.BaseStream.Length;

            SourceFile.BaseStream.Position = 0;
            CompareToFile.BaseStream.Position = 0;

            Operation.State = true;
            
            TimeMeas.Start();

            while ((FilePosition < SourceFileSize) && (FilePosition < CompareToFileSize))
            {
                if (SourceFile.ReadByte() != CompareToFile.ReadByte())
                {
                    Operation.State = false;
                    Operation.DataAddress = (UInt32)FilePosition;
                    break;
                }

                FilePosition++;
            }

            TimeMeas.Stop();
            SourceFile.Close();
            CompareToFile.Close();
            e.Result = e.Argument;
        }

        private void Background_ProgressChanged(object sender, ProgressChangedEventArgs e)
        {
            ShowOperationProgress(e.ProgressPercentage, e.UserState as string);
        }

        private void Background_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            BackOperation Result = e.Result as BackOperation;
            bool BackgroundStop = true;
            
            if ((Result.CurrentOperation == Reading) || (Result.CurrentOperation == Writing) || (Result.CurrentOperation == FileComparing) || (Result.CurrentOperation == VerifyAfterWrite))
                ShowOperationProgress(100, Result.CurrentOperation);
            InfoBox.Text += "\n" + OperationComplete + ExeTime + TimeMeas.ElapsedMilliseconds.ToString() + " мс\n";
            InfoBoxManager();

            if ((Result.InitOperation == Writing) && (VerifyAfterWriteCheck.IsChecked == true) && (Result.CurrentOperation == Writing))
            {
                BackgroundStop = false;
                Result.CurrentOperation = Reading;
                InfoBox.Text += Result.CurrentOperation;

                if (!ConnectToChip(NOT_DEINIT_AFTER_CONNECTION, false))
                    return;

                Result.CompareToFile = Result.SourceFile;
                Result.SourceFile = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetEntryAssembly().Location) + "\\RDC2-0026.bin";

                BinaryWriter SourceFile = new BinaryWriter(File.Create(Result.SourceFile));
                SourceFile.Close();

                TimeMeas.Reset();

                (sender as BackgroundWorker).DoWork -= WriteBackground_DoWork;
                (sender as BackgroundWorker).DoWork += ReadBackground_DoWork;
                (sender as BackgroundWorker).RunWorkerAsync(Result);
            }

            else if ((Result.CurrentOperation == Reading) && (((Result.InitOperation == Reading) && (CompareFilesCheck.IsChecked == true))
                                                           || ((Result.InitOperation == Writing) && (VerifyAfterWriteCheck.IsChecked == true))))                
            {
                BackgroundStop = false;
                
                if (Result.InitOperation == Reading)
                {
                    Result.CurrentOperation = FileComparing;
                    long SourceFileSize = new FileInfo(Result.SourceFile).Length;
                    long CompareToFileSize = new FileInfo(Result.CompareToFile).Length;

                    if (SourceFileSize == CompareToFileSize)
                        InfoBox.Text += CompareFileEqual;
                    else if (SourceFileSize < CompareToFileSize)
                        InfoBox.Text += CompareFileIsBig;
                    else
                        InfoBox.Text += CompareFileIsSmall;
                }

                else
                    Result.CurrentOperation = VerifyAfterWrite;

                InfoBox.Text += Result.CurrentOperation;

                TimeMeas.Reset();

                (sender as BackgroundWorker).ProgressChanged -= Background_ProgressChanged;
                (sender as BackgroundWorker).DoWork -= ReadBackground_DoWork;
                (sender as BackgroundWorker).DoWork += CompareFilesBackground_DoWork;
                (sender as BackgroundWorker).RunWorkerAsync(Result);
            }

            else if (Result.CurrentOperation == BlankChecking)
            {
                if (Result.State)
                    InfoBox.Text += MemoryIsBlank;
                else
                    InfoBox.Text += MemoryIsNotBlank + "0x" + Result.DataAddress.ToString("X8") + "\n";
            }

            else if ((Result.CurrentOperation == FileComparing) || (Result.CurrentOperation == VerifyAfterWrite))
            {
                if (Result.State)
                    InfoBox.Text += FilesIdentical;
                else
                    InfoBox.Text += FilesDifferent + "0x" + Result.DataAddress.ToString("X8") + "\n";
            }

            if (BackgroundStop)
            {
                if ((Result.InitOperation == Writing) && (VerifyAfterWriteCheck.IsChecked == true))
                    File.Delete(Result.SourceFile);

                ChipBorder.IsEnabled = true;
                OperationBorder.IsEnabled = true;
                InfoBox.Text += "\n";
                InfoBoxManager();
            }
        }

        private void ConnectCheckbutton_Click(object sender, RoutedEventArgs e)
        {
            ConnectToChip(DEINIT_AFTER_CONNECTION, true);
            InfoBox.Text += "\n";
            InfoBoxManager();
        }

        private void Erasebutton_Click(object sender, RoutedEventArgs e)
        {
            if (!ConnectToChip(NOT_DEINIT_AFTER_CONNECTION, true))
            {
                InfoBoxManager();
                return;
            }

            BackOperation MemoryOperation = new BackOperation { CurrentOperation = Erasing };

            InfoBox.Text += MemoryOperation.CurrentOperation;

            ChipBorder.IsEnabled = false;
            OperationBorder.IsEnabled = false;

            TimeMeas.Reset();

            byte[] USBPacket = new byte[USB_CMD_PACKET_SIZE];
            USBPacket[USB_REPORT_ID_POS] = USB_CMD_ID;
            USBPacket[USB_CMD_POS] = USB_CMD_MEMORY_ERASE;
            bool USBSuccess = USB_device.Write(USBPacket);

            BackgroundWorker EraseBackground = new BackgroundWorker();
            EraseBackground.DoWork += EraseBlankCheckBackground_DoWork;
            EraseBackground.RunWorkerCompleted += Background_RunWorkerCompleted;
            EraseBackground.RunWorkerAsync(MemoryOperation);
        }
        
        private void BlankCheckbutton_Click(object sender, RoutedEventArgs e)
        {
            if (!ConnectToChip(NOT_DEINIT_AFTER_CONNECTION, true))
            {
                InfoBoxManager();
                return;
            }

            BackOperation MemoryOperation = new BackOperation { CurrentOperation = BlankChecking };

            InfoBox.Text += MemoryOperation.CurrentOperation;

            ChipBorder.IsEnabled = false;
            OperationBorder.IsEnabled = false;

            TimeMeas.Reset();

            byte[] USBPacket = new byte[USB_CMD_PACKET_SIZE];
            USBPacket[USB_REPORT_ID_POS] = USB_CMD_ID;
            USBPacket[USB_CMD_POS] = USB_CMD_MEMORY_BLANK_CHECK;
            bool USBSuccess = USB_device.Write(USBPacket);

            BackgroundWorker BlankCheckBackground = new BackgroundWorker();
            BlankCheckBackground.DoWork += EraseBlankCheckBackground_DoWork;
            BlankCheckBackground.RunWorkerCompleted += Background_RunWorkerCompleted;
            BlankCheckBackground.RunWorkerAsync(MemoryOperation);
        }

        private void EraseBlankCheckBackground_DoWork(object sender, DoWorkEventArgs e)
        {
            BackOperation Operation = e.Argument as BackOperation;

            byte[] USBPacket = new byte[USB_CMD_PACKET_SIZE];
            bool USBSuccess;

            TimeMeas.Start();
            ShowProgressTimer.Start();

            byte ProgStatus = 0;
            while ((ProgStatus & PROG_USB_OPER_COMPLETE) != PROG_USB_OPER_COMPLETE)
            {
                USBPacket[USB_REPORT_ID_POS] = USB_CMD_ID;
                USBPacket[USB_CMD_POS] = USB_CMD_GET_STATE;
                USBSuccess = USB_device.Write(USBPacket);
                USBSuccess = USB_device.Read(USBPacket);
                ProgStatus = USBPacket[USB_START_DATA_POS];
            }

            TimeMeas.Stop();
            ShowProgressTimer.Stop();

            if (Operation.CurrentOperation == BlankChecking)
            {
                bool BlankState = false;
                UInt32 StartDataAdr = 0;
                if ((ProgStatus & MEMORY_BLANK) == MEMORY_BLANK)
                    BlankState = true;
                else
                {
                    for (byte i = 0; i < 4; i++)
                        StartDataAdr |= (byte)(USBPacket[USB_START_DATA_POS + 1 + i] << (8 * i));
                }

                Operation.State = BlankState;
                Operation.DataAddress = StartDataAdr;
            }

            e.Result = e.Argument;
        }
        
        private void CompareFilesChecked(object sender, RoutedEventArgs e)
        {
            FileToCompareToTextBox.IsEnabled = true;
            FileToCompareToButton.IsEnabled = true;
        }

        private void CompareFilesUnChecked(object sender, RoutedEventArgs e)
        {
            FileToCompareToTextBox.IsEnabled = false;
            FileToCompareToButton.IsEnabled = false;
        }

        private void ChooseFileToWriteFromButton_Click(object sender, RoutedEventArgs e)
        {
            string FileFilter = BinaryFileFilter;

            if (SigmaStudioFileCheck.IsChecked == true)
                FileFilter = SigmaStudioFileFilter;

            ChooseFile(FileToWriteFromTextBox, true, FileFilter);
        }

        private void ChooseFileToReadToButton_Click(object sender, RoutedEventArgs e)
        {
            ChooseFile(FileToReadToTextBox, false, BinaryFileFilter);
        }

        private void FileToCompareToButton_Click(object sender, RoutedEventArgs e)
        {
            string FileFilter = BinaryFileFilter;

            if (SigmaStudioFileCheck.IsChecked == true)
                FileFilter = SigmaStudioFileFilter;

            ChooseFile(FileToCompareToTextBox, true, FileFilter);
        }

        private void ReadStatusButton_Click(object sender, RoutedEventArgs e)
        {
            if (!ConnectToChip(NOT_DEINIT_AFTER_CONNECTION, true))
            {
                InfoBox.Text += "\n";
                InfoBoxManager();
                return;
            }

            InfoBox.Text += StatusReading;

            ChipBorder.IsEnabled = false;
            OperationBorder.IsEnabled = false;

            byte[] USBPacket = new byte[USB_CMD_PACKET_SIZE];
            USBPacket[USB_REPORT_ID_POS] = USB_CMD_ID;
            USBPacket[USB_CMD_POS] = USB_CMD_SPI_READ_STATUS;
            bool USBSuccess = USB_device.Write(USBPacket);
            USBSuccess = USB_device.Read(USBPacket);

            ReadStatusTextBox.Text = (USBPacket[USB_START_DATA_POS]).ToString();

            InfoBox.Text += OperationComplete + "\n\n";
            InfoBoxManager();

            ChipBorder.IsEnabled = true;
            OperationBorder.IsEnabled = true;
        }

        private void WriteStatusButton_Click(object sender, RoutedEventArgs e)
        {
            if (!ConnectToChip(NOT_DEINIT_AFTER_CONNECTION, true))
            {
                InfoBox.Text += "\n";
                InfoBoxManager();
                return;
            }

            byte NewStatus;
            if (!byte.TryParse(WriteStatusTextBox.Text, out NewStatus))
            {
                InfoBox.Text += "\n";
                InfoBoxManager();
                return;
            }

            ReadStatusTextBox.Clear();

            InfoBox.Text += StatusWriting;

            ChipBorder.IsEnabled = false;
            OperationBorder.IsEnabled = false;

            byte[] USBPacket = new byte[USB_CMD_PACKET_SIZE];
            USBPacket[USB_REPORT_ID_POS] = USB_CMD_ID;
            USBPacket[USB_CMD_POS] = USB_CMD_SPI_WRITE_STATUS;
            USBPacket[USB_START_DATA_POS] = NewStatus;
            bool USBSuccess = USB_device.Write(USBPacket);

            NewStatus = 0;
            while ((NewStatus & PROG_USB_OPER_COMPLETE) != PROG_USB_OPER_COMPLETE)
            {
                USBPacket[USB_REPORT_ID_POS] = USB_CMD_ID;
                USBPacket[USB_CMD_POS] = USB_CMD_GET_STATE;
                USBSuccess = USB_device.Write(USBPacket);
                USBSuccess = USB_device.Read(USBPacket);
                NewStatus = USBPacket[USB_START_DATA_POS];
            }

            InfoBox.Text += OperationComplete + "\n\n";
            InfoBoxManager();

            ChipBorder.IsEnabled = true;
            OperationBorder.IsEnabled = true;
        }

        private void ChooseFile(TextBox TextBoxItem, bool FileExistsCheck, string FileFilter)
        {
            string FileName = null;

            OpenFileDialog ChooseFile = new OpenFileDialog();
            ChooseFile.CheckFileExists = FileExistsCheck;
            ChooseFile.Filter = FileFilter;
            if (ChooseFile.ShowDialog() == true)
                FileName = ChooseFile.FileName;
            if (FileName != null)
                TextBoxItem.Text = FileName;
        }

        private bool ConnectToChip(bool DeInit, bool IsInfoEnabled)
        {
            if (IsInfoEnabled)
                InfoBox.Text += ConnectingToChip;

            byte[] USBPacket = new byte[USB_CMD_PACKET_SIZE];

            USBPacket[USB_REPORT_ID_POS] = USB_CMD_ID;
            USBPacket[USB_CMD_POS] = USB_CMD_MEMORY_INIT;
            USBPacket[MEMORY_TYPE_OFFSET] = (byte)(TypeСomboBox.SelectedIndex);

            if (USBPacket[MEMORY_TYPE_OFFSET] == I2C_EEPROM_FRAM)
                USBPacket[MEMORY_TYPE_OFFSET] = I2C_MEMORY;
            else if (USBPacket[MEMORY_TYPE_OFFSET] == SPI_EEPROM_FRAM)
                USBPacket[MEMORY_TYPE_OFFSET] = SPI_MEMORY_EEPROM;
            else if (USBPacket[MEMORY_TYPE_OFFSET] == SPI_FLASH)
                USBPacket[MEMORY_TYPE_OFFSET] = SPI_MEMORY_FLASH;
            else if (USBPacket[MEMORY_TYPE_OFFSET] == Microwire_EEPROM)
            {
                USBPacket[MEMORY_TYPE_OFFSET] = MICROWIRE_EEPROM_ORG_8;
                if (OrgСomboBox.SelectedIndex == 1)
                    USBPacket[MEMORY_TYPE_OFFSET] = MICROWIRE_EEPROM_ORG_16;
            }

            MemVolume = PageSizeByte[PageSizeСomboBox.SelectedIndex];
            if (TypeСomboBox.SelectedIndex == Microwire_EEPROM)
                MemVolume = 1;
            for (int i = 0; i < 2; i++)
                USBPacket[MEMORY_PAGE_SIZE_OFFSET + i] = (byte)(MemVolume >> (8 * i));

            int[] VolumeInBytes = new int[16];
            if ((TypeСomboBox.SelectedIndex == I2C_EEPROM_FRAM) || (TypeСomboBox.SelectedIndex == SPI_EEPROM_FRAM))
                VolumeInBytes = I2C_SPI_EEPROM_VolumeInBytes;
            else if (TypeСomboBox.SelectedIndex == SPI_FLASH)
                VolumeInBytes = SPI_FLASH_VolumeInBytes;
            else if (TypeСomboBox.SelectedIndex == Microwire_EEPROM)
                VolumeInBytes = MicrowireVolumeInBytes;

            MemVolume = VolumeInBytes[VolumeСomboBox.SelectedIndex];

            if (TypeСomboBox.SelectedIndex == I2C_EEPROM_FRAM)
            {
                USBPacket[MEMORY_ADDRESS_BYTES_OFFSET] = 1;
                USBPacket[MEMORY_ADDRESS_BITS_OFFSET] = 0;
                USBPacket[MEMORY_FREQUENCY_OFFSET] = (byte)FreqСomboBox.SelectedIndex;

                if (((MemVolume >= 512) && (MemVolume <= 2048)) || (MemVolume >= 131072))
                    USBPacket[MEMORY_ADDRESS_BITS_OFFSET] = 1;//здесь важно не значение, а что не равно нулю
                                                              //МК определяет точный адрес

                if (MemVolume > 2048)
                    USBPacket[MEMORY_ADDRESS_BYTES_OFFSET] = 2;
            }

            else if ((TypeСomboBox.SelectedIndex == SPI_EEPROM_FRAM) || (TypeСomboBox.SelectedIndex == SPI_FLASH))
            {
                USBPacket[MEMORY_FREQUENCY_OFFSET] = SPIFreqDevider[FreqСomboBox.SelectedIndex];
                USBPacket[MEMORY_SPI_CS_OFFSET] = SPIChipSel[ConnectСomboBox.SelectedIndex];

                if ((TypeСomboBox.SelectedIndex == SPI_EEPROM_FRAM) && (MemVolume <= 512))
                    USBPacket[MEMORY_ADDRESS_BYTES_OFFSET] = 1;
                else if ((TypeСomboBox.SelectedIndex == SPI_EEPROM_FRAM) && (MemVolume <= 65536))
                    USBPacket[MEMORY_ADDRESS_BYTES_OFFSET] = 2;
                else
                    USBPacket[MEMORY_ADDRESS_BYTES_OFFSET] = 3;

                USBPacket[MEMORY_ADDRESS_BITS_OFFSET] = 0;

                if (((TypeСomboBox.SelectedIndex == SPI_EEPROM_FRAM) && (MemVolume == 512)))
                    USBPacket[MEMORY_ADDRESS_BITS_OFFSET] = 1;
            }

            else if (TypeСomboBox.SelectedIndex == Microwire_EEPROM)
            {
                USBPacket[MEMORY_FREQUENCY_OFFSET] = MicrowireFreqDevider[FreqСomboBox.SelectedIndex];
                USBPacket[MEMORY_SPI_CS_OFFSET] = MicrowireChipSel[ConnectСomboBox.SelectedIndex];
                USBPacket[MEMORY_ADDRESS_BYTES_OFFSET] = 0;

                USBPacket[MEMORY_ADDRESS_BITS_OFFSET] = (byte)Microwire8BitOrgAddrBits[VolumeСomboBox.SelectedIndex];
                if (USBPacket[MEMORY_TYPE_OFFSET] == MICROWIRE_EEPROM_ORG_16)
                {
                    USBPacket[MEMORY_ADDRESS_BITS_OFFSET]--;
                    MemVolume /= 2;
                }
            }

            for (int i = 0; i < 4; i++)
                USBPacket[MEMORY_VOLUME_OFFSET + i] = (byte)(MemVolume >> (8 * i));
            if (USBPacket[MEMORY_TYPE_OFFSET] == MICROWIRE_EEPROM_ORG_16)
                MemVolume *= 2;

            USBPacket[MEMORY_START_ADDRESS_OFFSET] = 0;

            USBPacket[MEMORY_DEINIT_FLAG_OFFSET] = 0;

            if (DeInit)
                USBPacket[MEMORY_DEINIT_FLAG_OFFSET] = 1;

            Boolean USBSuccess = USB_device.Write(USBPacket);

            USBSuccess = USB_device.Read(USBPacket);

            if ((USBSuccess) && (USBPacket[USB_START_DATA_POS] == MEMORY_PRESENT))
            {
                if ((IsInfoEnabled) && (TypeСomboBox.SelectedIndex != Microwire_EEPROM))
                {
                    InfoBox.Text += ChipDetected;

                    if (TypeСomboBox.SelectedIndex == SPI_FLASH)
                    {
                        InfoBox.Text += Read_JEDEC_ID;
                        if ((USBPacket[USB_START_DATA_POS + 2] != 0xFF) && (USBPacket[USB_START_DATA_POS + 2] != 0x00))
                        {
                            InfoBox.Text += ManufactureID + "0x" + (USBPacket[USB_START_DATA_POS + 2]).ToString("X2") + "\n"
                                           + DeviceID + "0x" + (USBPacket[USB_START_DATA_POS + 3]).ToString("X2")
                                                             + (USBPacket[USB_START_DATA_POS + 4]).ToString("X2") + "\n";
                        }

                        else
                            InfoBox.Text += JEDEC_ID_NotSupported;
                    }
                }

                return true;
            }

            else
            {
                if ((IsInfoEnabled) && (TypeСomboBox.SelectedIndex != Microwire_EEPROM))
                    InfoBox.Text += ChipNotDetected;
                return false;
            }
        }

        private void ShowOperationProgress(int ProgressPercent, string MemoryOperation)
        {
            if ((InfoBox.Text.LastIndexOf(MemoryOperation) + MemoryOperation.Length) < InfoBox.Text.Length)
                InfoBox.Text = InfoBox.Text.Remove(InfoBox.Text.LastIndexOf(MemoryOperation) + MemoryOperation.Length);
            InfoBox.Text += ProgressPercent.ToString() + "%";
        }

        private void ShowProgressTimer_Tick(object sender, EventArgs e)
        {
            InfoBox.Text += " . ";
        }

        private void InfoBoxManager()
        {
            if (InfoBox.LineCount > MAX_INFO_STRING_COUNT)
            {
                int StringsToDelete = InfoBox.LineCount - MAX_INFO_STRING_COUNT;

                for (int i = 0; i < StringsToDelete; i++)
                    InfoBox.Text = InfoBox.Text.Remove(0, InfoBox.GetLineLength(0));
            }

            InfoBox.ScrollToLine(InfoBox.LineCount - 1);
        }

        public class BackOperation
        {
            public string InitOperation { get; set; }
            public string CurrentOperation { get; set; }
            public string SourceFile { get; set; }
            public string CompareToFile { get; set; }
            public bool State { get; set; }
            public UInt32 DataAddress { get; set; }
        }

        private void WindowClosing(object sender, CancelEventArgs e)
        {
            USB_device.Close();
            string DefaultFilePath = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetEntryAssembly().Location) + "\\DefaultChip.txt";
            SaveChip(DefaultFilePath);
        }

        private void SaveChip(string FilePath)
        {
            using (StreamWriter FileToWriteTo = File.CreateText(FilePath))
            {
                FileToWriteTo.WriteLine(ChipNameString + ChipName.Text);
                FileToWriteTo.WriteLine(ManufactureString + Manufacture.Text);
                FileToWriteTo.WriteLine(TypeString + TypeСomboBox.SelectedItem);
                FileToWriteTo.WriteLine(VolumeString + VolumeСomboBox.SelectedItem);
                if (!OrgСomboBox.IsEnabled)
                    FileToWriteTo.WriteLine(PageSizeString + PageSizeСomboBox.SelectedItem);
                else
                    FileToWriteTo.WriteLine(OrganizationString + OrgСomboBox.SelectedItem);
                FileToWriteTo.WriteLine(FrequencyString + FreqСomboBox.SelectedItem);
                FileToWriteTo.WriteLine(ConnectionString + ConnectСomboBox.SelectedItem);
            }
        }

        private void ChooseChip(string FilePath)
        {
            using (StreamReader FileToReadFrom = File.OpenText(FilePath))
            {
                const byte TextItemsCount = 2;
                const byte ComboBoxItemsCount = 5;

                TextBox[] TextItem = new TextBox[TextItemsCount];
                for (int item = 0; item < TextItemsCount; item++)
                    TextItem[item] = new TextBox();
                TextItem[0] = ChipName;
                TextItem[1] = Manufacture;

                ComboBox[] ComboBoxItems = new ComboBox[ComboBoxItemsCount];
                for (int item = 0; item < ComboBoxItemsCount; item++)
                    ComboBoxItems[item] = new ComboBox();
                ComboBoxItems[0] = TypeСomboBox;
                ComboBoxItems[1] = VolumeСomboBox;
                ComboBoxItems[2] = PageSizeСomboBox;
                ComboBoxItems[3] = FreqСomboBox;
                ComboBoxItems[4] = ConnectСomboBox;

                string[] ParamStrings = { ChipNameString, ManufactureString, TypeString, VolumeString, PageSizeString, FrequencyString, ConnectionString, };

                bool OpenSuccess = true;
                String NewLine = "";

                for (int i = 0; i < (TextItemsCount + ComboBoxItemsCount); i++)
                {
                    if (FileToReadFrom.Peek() >= 0)
                        NewLine = FileToReadFrom.ReadLine();
                    else
                    {
                        OpenSuccess = false;
                        break;
                    }

                    if (i < TextItemsCount)
                    {
                        if (NewLine.IndexOf(ParamStrings[i]) != -1)
                            TextItem[i].Text = NewLine.Substring(ParamStrings[i].Length, NewLine.Length - ParamStrings[i].Length);
                        else
                            TextItem[i].Clear();
                    }

                    else
                    {
                        if (NewLine.IndexOf(ParamStrings[i]) == -1)
                        {
                            OpenSuccess = false;
                            break;
                        }

                        string ParamValueString = NewLine.Substring(ParamStrings[i].Length, NewLine.Length - ParamStrings[i].Length);
                        int ParamValueIndex = Array.IndexOf(ComboBoxItems[i - 2].ItemsSource as String[], ParamValueString);
                        if (ParamValueIndex == -1)
                        {
                            OpenSuccess = false;
                            break;
                        }

                        ComboBoxItems[i - 2].SelectedIndex = ParamValueIndex;

                        if ((i == 2) && (TypeСomboBox.SelectedIndex == Microwire_EEPROM))
                        {
                            ComboBoxItems[2] = OrgСomboBox;
                            ParamStrings[4] = OrganizationString;
                        }
                    }
                }

                if (!OpenSuccess)
                {
                    ChipName.Clear();
                    Manufacture.Clear();
                    TypeСomboBox.SelectedIndex = 0;
                }
            }
        }

        private void SigmaStudioFileConverter(string SigmaStudioHEXFile, string SigmaStudioBINFile)
        {
            BinaryReader InputFile = new BinaryReader(File.Open(SigmaStudioHEXFile, FileMode.Open));

            long FileLength = InputFile.BaseStream.Length;

            BinaryWriter OutputFile = new BinaryWriter(File.Create(SigmaStudioBINFile));

            InputFile.BaseStream.Position = 0;
            OutputFile.BaseStream.Position = 0;

            while (InputFile.BaseStream.Position < FileLength)
            {
                byte Data = InputFile.ReadByte();

                if (Data == 0x78) //начало данных 'x'
                {
                    Data = InputFile.ReadByte();
                    byte MSBHalf = ParseChar(Data);
                    Data = InputFile.ReadByte();
                    byte LSBHalf = ParseChar(Data);

                    if ((MSBHalf != 0xFF) && (LSBHalf != 0xFF))
                        OutputFile.Write((byte)((MSBHalf << 4) + LSBHalf));
                }
            }

            InputFile.Close();
            OutputFile.Close();
        }

        private byte ParseChar(byte InChar)
        {
            if ((InChar >= 0x30) && (InChar <= 0x39))
                return (byte)(InChar - 0x30);
            else if ((InChar >= 0x41) && (InChar <= 0x46))
                return (byte)(InChar - 0x41 + 10);
            else if ((InChar >= 0x61) && (InChar <= 0x66))
                return (byte)(InChar - 0x61 + 10);
            else
                return 0xFF;
        }
    }
}


