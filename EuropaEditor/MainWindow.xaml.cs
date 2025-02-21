using EuropaEditor.GameProject.Backend;
using System;
using System.Collections.Generic;
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

namespace EuropaEditor
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public static string EuropaPath = @"D:\Visual Studio Projects\Europa";

        public MainWindow()
        {
            InitializeComponent();
            Loaded += OnMainWindowLoaded;
            Closed += OnMainWindowClosed;
        }

        private void OnMainWindowClosed(object sender, EventArgs e)
        {
            Loaded -= OnMainWindowLoaded;
            Project.CurrentProject?.Unload();
        }

        private void OnMainWindowLoaded(object sender, RoutedEventArgs e)
        {
            Loaded -= OnMainWindowLoaded;
            GetEnginePath();
            ProjectBrowserDialogueControl();
        }

        private void GetEnginePath()
        {
            string europaPath = Environment.GetEnvironmentVariable("EUROPA_ENGINE", EnvironmentVariableTarget.User);
            if (europaPath == null || !Directory.Exists(System.IO.Path.Combine(europaPath, @"Engine\EngineAPI")))
            {
                var enginePathDialog = new EnginePathDialog();
                if (enginePathDialog.ShowDialog() == true)
                {
                    EuropaPath = enginePathDialog.EuropaPath;
                    Environment.SetEnvironmentVariable("EUROPA_ENGINE", EuropaPath.ToUpper(), EnvironmentVariableTarget.User);
                }
                else
                {
                    Application.Current.Shutdown();
                }
            }
            else
            {
                EuropaPath = europaPath;
            }
        }

        private void ProjectBrowserDialogueControl()
        {
            var projectBrowserDialogue = new ProjectBrowserDialogue();
            if (projectBrowserDialogue.ShowDialog() == false || projectBrowserDialogue.DataContext == null)
            {
                Application.Current.Shutdown();
            }
            else
            {
                Project.CurrentProject?.Unload();
                DataContext = projectBrowserDialogue.DataContext;
            }
        }
    }
}
