using EuropaEditor.GameProject.Backend;
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

namespace EuropaEditor
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
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
            ProjectBrowserDialogueControl();
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
