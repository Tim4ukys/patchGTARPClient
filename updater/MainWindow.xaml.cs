using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
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
using updater.pages;

namespace updater
{
    public partial class MainWindow : Window
    {
        private int _curPage = 0;
        private enum PagesItems : int
        {
            er_finish = -1,
            start = 0,
            loading,
            finish
        };
        private const int MAX_PAGES = 2;

        public MainWindow()
        {
            InitializeComponent();
            MainFrame.Content = new start();
        }

        public void enableNext()
        {
            Dispatcher.Invoke(() =>
            {
                btn_next.IsEnabled = true;
            });
        }
        private bool _closeNextWithErr = false;
        public void errEndNext()
        {
            Dispatcher.Invoke(() =>
            {
                btn_next.Content = "Закрыть";
                btn_next.IsEnabled = true;
                _closeNextWithErr = true;
            });
        }

        private void pageUpdate()
        {
            switch ((PagesItems)_curPage)
            {
                case PagesItems.start:
                    MainFrame.NavigationService.Navigate(new start());
                    break;
                case PagesItems.loading:
                    MainFrame.NavigationService.Navigate(new loading(this));
                    btn_next.IsEnabled = false;
                    break;
                case PagesItems.finish:
                    MainFrame.NavigationService.Navigate(new finish());
                    btn_next.Content = "Закрыть";
                    break;
                case PagesItems.er_finish:
                    //MainFrame.NavigationService.Navigate(new er_finish());
                    break;
                default:
                    throw new Exception("_curPage = какой-то залупе, а не PagesItems");
            }
        }

        private void btn_next_Click(object sender, RoutedEventArgs e)
        {
            if (++_curPage > MAX_PAGES || _closeNextWithErr) 
                Close();
            else 
                pageUpdate();
        }

        #region Загаловок окна
        private void PART_CLOSE_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }
        private void PART_MINIMIZE_Click(object sender, RoutedEventArgs e)
        {
            this.WindowState = WindowState.Minimized;
        }
        private void Title_MovedWindow(object sender, MouseButtonEventArgs e)
        {
            if (e.ChangedButton == MouseButton.Left)
            {
                this.DragMove();
            }
        }
        #endregion
    }
}
