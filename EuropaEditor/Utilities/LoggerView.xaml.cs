using EuropaEditor.Utilities;
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

namespace EuropaEditor.Utilities
{
    /// <summary>
    /// Interaction logic for LoggerView.xaml
    /// </summary>
    public partial class LoggerView : UserControl
    {
        public LoggerView()
        {
            InitializeComponent();
            Logger.Log(MessageType.Info, "Information message");
            Logger.Log(MessageType.Warning, "Warning message");
            Logger.Log(MessageType.Error, "Information message");
        }

        private void OnClear_Button_Click(object sender, RoutedEventArgs e)
        {
            Logger.Clear();
        }

        private void OnMessageFilter_Button_Click(object sender, RoutedEventArgs e)
        {
            var filter = 0x0;
            if (toggleInfo.IsChecked == true)
                filter |= (int)MessageType.Info;
            if(toggleWarnings.IsChecked == true)
                filter |= (int)MessageType.Warning;
            if (toggleErrors.IsChecked == true)
                filter |= (int)MessageType.Error;
            Logger.SetMessageFilter(filter);
        }
    }
}
