using System;
using System.Collections.Generic;
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
using System.Text.Json;
using System.Text.Json.Nodes;
using System.Diagnostics;

namespace updater.pages
{
    /// <summary>
    /// Логика взаимодействия для finish.xaml
    /// </summary>
    public partial class finish : Page
    {
        private static List<string> s_arrChangeLog = new List<string>();
        private static string s_vers = "";
        public static void getChangeLog(string strJsChangeLog, string vers)
        {
            JsonNode json = JsonNode.Parse(strJsChangeLog);
            foreach (var c in json[vers].AsArray())
            {
                var win1251 = Encoding.GetEncoding(1251);
                byte[] bytes = win1251.GetBytes((string)c);
                s_arrChangeLog.Add(Encoding.UTF8.GetString(bytes));
            }
            s_vers = vers;
        }

        public finish()
        {
            InitializeComponent();
            changeLog_header.Content = $"Changelog(v{s_vers}):";
            int i = 1;
            foreach (var s in s_arrChangeLog)
            {
                changeLog.Text += $"{i++}) {s}\r\n";
            }
        }

        private void Hyperlink_RequestNavigate(object sender, RequestNavigateEventArgs e)
        {
            Process.Start(new ProcessStartInfo(e.Uri.AbsoluteUri));
            e.Handled = true;
        }
    }
}
