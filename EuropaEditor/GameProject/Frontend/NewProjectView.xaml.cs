using EuropaEditor.GameProject.Backend;
using System;
using System.Collections.Generic;
using System.Diagnostics;
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

namespace EuropaEditor.GameProject
{
    /// <summary>
    /// Interaction logic for NewProjectView.xaml
    /// </summary>
    public partial class NewProjectView : UserControl
    {
        public NewProjectView()
        {
            InitializeComponent();
        }

        private void OnCreateButton_Click(object sender, RoutedEventArgs e)
        {
            var newProject = DataContext as NewProject;
            var projectPath = newProject.CreateProject(templatesListBox.SelectedItem as ProjectTemplate);
            bool dialogResult = false;
            var window = Window.GetWindow(this);
            if (!string.IsNullOrEmpty(projectPath))
            {
                dialogResult = true;
                OpenProject.Open(new ProjectData() { ProjectName = newProject.ProjectName, ProjectPath = projectPath });
            }
            window.DialogResult = dialogResult;
            Project project = new Project(newProject.ProjectName, projectPath);
            window.DataContext = project;
            window.Close();
        }
    }
}
