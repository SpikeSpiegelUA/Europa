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
        public string TemplatePath { get; internal set; }
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
                    ValidateProjectPath();
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
                    ValidateProjectPath();
                    OnPropertyChanged(nameof(ProjectPath));
                }
            }
        }

        private bool _isValid = false;

        public bool IsValid
        {
            get => _isValid;
            set
            {
                if(_isValid != value)
                {
                    _isValid = value;
                    OnPropertyChanged(nameof(IsValid));
                }
            }
        }

        private string _errorMessage = string.Empty;
        public string ErrorMessage
        {
            get => _errorMessage;
            set
            {
                if (_errorMessage != value)
                {
                    _errorMessage = value;
                    OnPropertyChanged(nameof(ErrorMessage));
                }
            }
        }

        private ObservableCollection<ProjectTemplate> _projectTemplates = new ObservableCollection<ProjectTemplate>();
        public ReadOnlyObservableCollection<ProjectTemplate> ProjectTemplates { get; }

        private bool ValidateProjectPath()
        {
            var path = ProjectPath;
            if (!path.Last().Equals(Path.DirectorySeparatorChar))
                path += Path.DirectorySeparatorChar;
            path += $@"{ProjectName}\";

            IsValid = false;
            if (string.IsNullOrWhiteSpace(ProjectName.Trim()))
            {
                ErrorMessage = "Type in a project name.";
            }
            else if(ProjectName.IndexOfAny(Path.GetInvalidFileNameChars()) != -1)
            {
                ErrorMessage = "Invalid character(s) used in a project name.";
            }
            else if (string.IsNullOrWhiteSpace(path.Trim()))
            {
                ErrorMessage = "Select a valid project folder.";
            }
            else if (path.IndexOfAny(Path.GetInvalidPathChars()) != -1)
            {
                ErrorMessage = "Invalid character(s) used in a project path.";
            }
            else if (Directory.Exists(path) && Directory.EnumerateFileSystemEntries(path).Any())
            {
                ErrorMessage = "Selected project folder already exists and is not empty.";
            }
            else
            {
                ErrorMessage = string.Empty;
                IsValid = true;
            }
            return IsValid;
        }

        public string CreateProject(ProjectTemplate projectTemplate)
        {
            ValidateProjectPath();
            if(!IsValid)
                return string.Empty;
            if (!ProjectPath.Last().Equals(Path.DirectorySeparatorChar))
                ProjectPath += Path.DirectorySeparatorChar;
            var path = ProjectPath + $@"{ProjectName}\";

            try
            {
                if(!Directory.Exists(path))
                    Directory.CreateDirectory(path);
                foreach(string folder in projectTemplate.ProjectFolders)
                {
                    Directory.CreateDirectory(Path.GetFullPath(Path.Combine(Path.GetDirectoryName(path), folder)));
                }
                var EuropaDirectoryInfo = new DirectoryInfo(path + @".europa\");
                EuropaDirectoryInfo.Attributes |= FileAttributes.Hidden;
                File.Copy(projectTemplate.IconFilePath, Path.GetFullPath(Path.Combine(EuropaDirectoryInfo.FullName, "Icon.png")));
                File.Copy(projectTemplate.ScreenshotFilePath, Path.GetFullPath(Path.Combine(EuropaDirectoryInfo.FullName, "Screenshot.png")));

                var projectXML = File.ReadAllText(projectTemplate.ProjectFilePath);
                projectXML = string.Format(projectXML, ProjectName, path);
                var projectPath = Path.GetFullPath(Path.Combine(path, $"{ProjectName}{Project.Extension}"));
                File.WriteAllText(projectPath, projectXML);
          
                ProjectData projectData = new ProjectData();
                projectData.ProjectName = ProjectName;
                projectData.ProjectPath = path;
                projectData.Icon = projectTemplate.Icon;
                projectData.Screenshot = projectTemplate.Screenshot;
                projectData.DateAndTime = DateTime.Now;
                OpenProject.AddNewProject(projectData);
                CreateMSVCSolution(projectTemplate, path);
                return path;
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex);
                Logger.Log(MessageType.Error, $"Failed to create {ProjectName} project");
                throw;
            }
        }

        private void CreateMSVCSolution(ProjectTemplate projectTemplate, string path)
        {
            Debug.Assert(File.Exists(Path.Combine(projectTemplate.TemplatePath, "MSVCSolution.xml")));
            Debug.Assert(File.Exists(Path.Combine(projectTemplate.TemplatePath, "MSVCProject.xml")));

            string engineAPIPath = Path.Combine(MainWindow.EuropaPath, @"Engine\EngineAPI\");
            Debug.Assert(Directory.Exists(engineAPIPath));

            string solutionZeroParameter = ProjectName;
            string solutionFirstParameter = '{' + Guid.NewGuid().ToString().ToUpper() + '}';
            string solutionSecondParameter = '{' + Guid.NewGuid().ToString().ToUpper() + '}';

            string solution = File.ReadAllText(Path.Combine(projectTemplate.TemplatePath, "MSVCSolution.xml"));
            solution = string.Format(solution, solutionZeroParameter, solutionFirstParameter, solutionSecondParameter);
            File.WriteAllText(Path.GetFullPath(Path.Combine(path, $"{solutionZeroParameter}.sln")), solution);

            string projectZeroParameter = ProjectName;
            string projectFirstParameter = '{' + Guid.NewGuid().ToString().ToUpper() + '}';
            string projectSecondParameter = engineAPIPath;
            string projectThirdParameter = MainWindow.EuropaPath;


            string project = File.ReadAllText(Path.Combine(projectTemplate.TemplatePath, "MSVCProject.xml"));
            project = string.Format(project, projectZeroParameter, projectFirstParameter, projectSecondParameter, projectThirdParameter);
            File.WriteAllText(Path.GetFullPath(Path.Combine(path, $@"GameCode\{projectZeroParameter}.vcxproj")), project);
        }

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
                    template.TemplatePath = Path.GetDirectoryName(file);
                    template.IconFilePath = Path.Combine(template.TemplatePath, "Icon.png");
                    template.Icon = File.ReadAllBytes(template.IconFilePath);
                    template.ScreenshotFilePath = Path.Combine(template.TemplatePath, "Screenshot.png");
                    template.Screenshot = File.ReadAllBytes(template.ScreenshotFilePath);
                    template.ProjectFilePath = Path.Combine(template.TemplatePath, template.ProjectFile);
                    _projectTemplates.Add(template);
                }
                ValidateProjectPath();
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.Log(MessageType.Error, $"Failed to read project templates");
                throw;
            }
        }
    }
}
