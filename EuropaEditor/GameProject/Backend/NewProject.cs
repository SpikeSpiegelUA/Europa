using System;
using System.Collections.Generic;
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
    }
    internal class NewProject : ViewModelBase
    {
        //TODO: get the path from the installation location.
        private readonly string _templatePath = @"..\..\EuropaEditor\ProjectTemplates\";
        private string _name = "New project";
        public string Name
        {
            get => _name;
            set
            {
                if (!_name.Equals(value))
                {
                    _name = value;
                    OnPropertyChanged(nameof(Path));
                }
            }
        }

        private string _path = $@"{Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments)}\EuropaProject\";
        public string Path
        {
            get => _path;
            set
            {
                if (!_path.Equals(value))
                {
                    _path = value;
                    OnPropertyChanged(nameof(Path));
                }
            }
        }
        public NewProject()
        {
            try
            {
                var templatesFiles = Directory.GetFiles(_templatePath, "template.xml", SearchOption.AllDirectories);
                Debug.Assert(templatesFiles.Any());
                foreach(var file in templatesFiles)
                {
                    var template = new ProjectTemplate()
                    {
                        ProjectType = "Empty Project",
                        ProjectFile = "project.europa",
                        ProjectFolders = new List<string> { ".Europa", "Content", "GameCode" }
                    }; 

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
