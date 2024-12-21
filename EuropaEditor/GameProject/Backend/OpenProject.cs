using EuropaEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Shapes;

namespace EuropaEditor.GameProject.Backend
{
    [DataContract]
    public class ProjectData
    {
        [DataMember]
        public string ProjectName { get; set; }
        [DataMember]
        public string ProjectPath { get; set; }
        [DataMember]
        public DateTime DateAndTime { get; set; }
        public string FullPath { get => $"{ProjectPath}{ProjectName}{Project.Extension}"; }
        public byte[] Icon { get; set; }
        public byte[] Screenshot { get; set; }
    }

    [DataContract]
    public class ProjectDataList
    {
        [DataMember]
        public List<ProjectData> Projects { get; set; }
    }

    public class OpenProject
    {
        private static readonly string _applicationDataPath = $@"{Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData)}\EuropaEditor\";
        private static readonly string _projectDataPath;
        private static readonly ObservableCollection<ProjectData> _projects = new ObservableCollection<ProjectData>();
        public static ReadOnlyObservableCollection<ProjectData> Projects { get; }

        private static void ReadProjectData()
        {
            if (File.Exists(_projectDataPath))
            {
                var projects = Serializer.FromFile<ProjectDataList>(_projectDataPath).Projects.OrderByDescending(x => x.DateAndTime);
                _projects.Clear();
                foreach(ProjectData projectData in projects)
                {
                    if (File.Exists(projectData.FullPath))
                    {
                        projectData.Icon = File.ReadAllBytes($@"{projectData.ProjectPath}\.europa\Icon.png");
                        projectData.Screenshot = File.ReadAllBytes($@"{projectData.ProjectPath}\.europa\Screenshot.png");
                        _projects.Add(projectData);
                    }
                }
            }
        }

        private static void WriteProjectData()
        {
            var projects = _projects.OrderBy(x => x.DateAndTime).ToList();
            Serializer.ToFile(new ProjectDataList() { Projects = projects }, _projectDataPath);
        }
        
        public static void AddNewProject(ProjectData newProject)
        {
            _projects.Add(newProject);
            WriteProjectData();
        }

        internal static Project Open(ProjectData projectData)
        {
            ReadProjectData();
            var project = _projects.FirstOrDefault(x => x.FullPath == projectData.FullPath);
            if(project != null)
            {
                project.DateAndTime = DateTime.Now;
            }
            else
            {
                project = projectData;
                project.DateAndTime = DateTime.Now;
                _projects.Add(project);
            }
            WriteProjectData();

            return Project.Load(projectData.FullPath);
        }

        static OpenProject()
        {
            try
            {
                if(!Directory.Exists(_applicationDataPath))
                    Directory.CreateDirectory(_applicationDataPath);
                _projectDataPath = $@"{_applicationDataPath}ProjectData.xml";
                Projects = new ReadOnlyObservableCollection<ProjectData>(_projects);
                ReadProjectData();
            }
            catch(Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.Log(MessageType.Error, $"Failed to read projects data");
                throw;
            }
        }
    }
}
