using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Reflection.Emit;
using System.Text;
using System.Threading;
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
using System.Text.Json;
using System.Text.Json.Nodes;
using System.Windows.Interop;
using System.IO;
using System.IO.Compression;
using System.Security.Cryptography;

namespace updater.pages
{
    /// <summary>
    /// Логика взаимодействия для loading.xaml
    /// </summary>
    public partial class loading : Page
    {
        private Thread thread;

        private bool endedMsg = true;
        private void addMsg(string msg)
        {
            Dispatcher.Invoke(() =>
            {
                if (endedMsg)
                    logBox.Text += $"[{DateTime.Now:HH:mm:ss}]: ";
                logBox.Text += msg;
                endedMsg = msg.EndsWith("\r\n");
            });
        }

        private void addMsgErr(string text)
        {

        }

        string[] goodFiles = new string[]
        {
            "!000patchGTARPClientByTim4ukys.ASI",
            "!000patchGTARPClientByTim4ukys.pdb",
            "gtarp_clientside.asi"
        };

        public loading(MainWindow mainWindow)
        {
            InitializeComponent();

            thread = new Thread(() =>
            {
                try
                {
                    ServicePointManager.Expect100Continue = true;
                    ServicePointManager.SecurityProtocol = SecurityProtocolType.Tls12;
                    addMsg("Подключение к github ... ");
                    string vers = "";
                    using (WebClient client = new WebClient())
                    {
                        var or_js = client.DownloadString("https://raw.githubusercontent.com/Tim4ukys/patchGTARPClient/beta/update.json");
                        JsonNode json = JsonNode.Parse(or_js);
                        vers = json["vers"].ToString();
                        finish.getChangeLog(client.DownloadString("https://raw.githubusercontent.com/Tim4ukys/patchGTARPClient/beta/news.json"),
                            vers);
                        Dispatcher.Invoke(new Action(() => { progressBar.Value += 10; }));
                    }
                    addMsg("последняя публичная версия [" + vers + "]\r\n");
                    addMsg("Скачивание архива ... ");

                    bool finishWithErr = false;
                    using (WebClient client = new WebClient())
                    {
                        client.DownloadProgressChanged += (object sender, DownloadProgressChangedEventArgs e) =>
                        {
                            Dispatcher.Invoke(new Action(() =>
                            {
                                double bytesIn = double.Parse(e.BytesReceived.ToString());
                                double totalBytes = double.Parse(e.TotalBytesToReceive.ToString());
                                progressBar.Value = 10 + int.Parse(Math.Truncate(bytesIn / totalBytes * 100).ToString());
                            }));
                        };
                        client.DownloadFileCompleted += (object sender, System.ComponentModel.AsyncCompletedEventArgs e) =>
                        {
                            if (e.Error != null)
                            {
                                finishWithErr = true;
                                if (!endedMsg)
                                {
                                    addMsg("Прервано\r\n");
                                }
                                addMsg("<ERROR> " + e.Error.ToString() + "\r\n");
                                mainWindow.errEndNext();
                            }
                        };
                        if (!Directory.Exists(".temp"))
                            Directory.CreateDirectory(".temp");
                        client.DownloadFileAsync(new Uri(@"https://github.com/Tim4ukys/patchGTARPClient/releases/download/v" + vers + "/patchGTARPClientByTim4ukys.zip"),
                            @".temp\patchGTARPClientByTim4ukys.zip");

                        while (client.IsBusy) { }
                    }
                    Thread.Sleep(250);
                    if (finishWithErr)
                        return;

                    addMsg("успешно\r\n");

                    addMsg("Получение данных из архива:\r\n");
                    using (ZipArchive archive = ZipFile.Open(@".temp\patchGTARPClientByTim4ukys.zip", ZipArchiveMode.Read))
                    {
                        foreach (var s in archive.Entries)
                        {
                            addMsg($"{s.Name}: Len=0x{s.Length:X}; Date={s.LastWriteTime};\r\n");

                            foreach (var g in goodFiles)
                            {
                                if (s.Name != g) continue;

                                addMsg("Разархивация " + s.Name + " ... ");
                                s.ExtractToFile(s.Name, true);
                                addMsg("успешно\r\n");
                                Dispatcher.Invoke(new Action(() =>
                                {
                                    progressBar.Value += 10;
                                }));
                            }
                        }
                    }
                    addMsg("Разархивация файлов прошла успешно!\r\n");

                    mainWindow.enableNext();
                }
                catch (TaskCanceledException) { }
                catch (WebException ec) {
                    if (!endedMsg)
                    {
                        addMsg("Прервано\r\n");
                    }
                    addMsg("<ERROR> " + ec.ToString() + "\r\n");
                    mainWindow.errEndNext();
                }
                catch (Exception ec)
                {
                    if (!endedMsg)
                    {
                        addMsg("Прервано\r\n");
                    }
                    addMsg("<ERROR> " + ec.ToString() + "\r\n");
                    mainWindow.errEndNext();
                }
                finally
                {
                    if (Directory.Exists(".temp")) 
                        Directory.Delete(".temp", true);
                }
            });
            thread.Start();
        }

        private void logBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            logBox.ScrollToEnd();
        }
    }


}
