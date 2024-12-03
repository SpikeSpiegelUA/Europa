using EuropaEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;

namespace EuropaEditor.GameProject.Backend
{
    [DataContract]
    public class ProjectTemplate
    {
        [DataMember]
        public string ProjectType { get; set; }
        [DataMember]
        public string ProjectFile { get; set; }
        [DataMember]
        public List<string> ProjectFolders { get; set; }

        public byte[] Icon { get; set; }
        public byte[] Screenshot { get; set; }

        public string IconFilePath { get; set; }
        public string ScreenshotFilePath { get; set; }
        public string ProjectFilePath { get; set; }
    }

    internal class NewProject : ViewModelBase
    {
        //TODO: get the path from the installation location.
        private readonly string _templatePath = @"..\..\EuropaEditor\ProjectTemplates\";
        private string _projectName = "New project";
        public string ProjectName
        {
            get => _projectName;
            set
            {
                if (!_projectName.Equals(value))
                {
                    _projectName = value;
                    OnPropertyChanged(nameof(ProjectPath));
                }
            }
        }

        private string _projectPath = $@"{Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments)}\EuropaProject\";
        public string ProjectPath
        {
            get => _projectPath;
            set
            {
                if (!_projectPath.Equals(value))
                {
                    _projectPath = value;
                    OnPropertyChanged(nameof(ProjectPath));
                }
            }
        }

        private ObservableCollection<ProjectTemplate> _projectTemplates = new ObservableCollection<ProjectTemplate>();
        public ReadOnlyObservableCollection<ProjectTemplate> ProjectTemplates { get; }

        public NewProject()
        {
            ProjectTemplates = new ReadOnlyObservableCollection<ProjectTemplate>(_projectTemplates);
            try
            {
                var templatesFiles = Directory.GetFiles(_templatePath, "template.xml", SearchOption.AllDirectories);
                Debug.Assert(templatesFiles.Any());
                foreach(var file in templatesFiles)
                {
                    var template = Serializer.FromFile<ProjectTemplate>(file);
                    template.IconFilePath = Path.Combine(Path.GetDirectoryName(file), "Icon.png");
                    template.Icon = File.ReadAllBytes(template.IconFilePath);
                    template.ScreenshotFilePath = Path.Combine(Path.GetDirectoryName(file), "Screenshot.png");
                    template.Screenshot = File.ReadAllBytes(template.ScreenshotFilePath);
                    template.ProjectFilePath = Path.Combine(Path.GetDirectoryName(file), template.ProjectFile);
                    _projectTemplates.Add(template);
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                //TODO: log error.
            }
        }
    }
}
