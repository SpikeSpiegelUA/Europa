using EuropaEditor.DLLWrapper;
using EuropaEditor.GameDev;
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
using System.Windows;
using System.Windows.Input;

namespace EuropaEditor.GameProject.Backend
{
    [DataContract(Name = "Game")]
    internal class Project : ViewModelBase
    {
        public enum BuildConfiguration
        {
            Debug,
            DebugEditor,
            Release,
            ReleaseEditor,
        }

        public static string Extension = ".europa";
        [DataMember]
        public string Name { get; private set; } = "New Project";
        [DataMember]
        public string Path { get; private set; }
        public string FullPath => $@"{Path}{Name}{Extension}";
        public string Solution => $@"{Path}{Name}.sln";

        private static readonly string[] _buildConfigurationNames = new string[]
        {
            "Debug",
            "DebugEditor",
            "Release",
            "ReleaseEditor"
        };

        private int _buildConfig;
        [DataMember]
        public int BuildConfig
        {
            get => _buildConfig;
            set
            {
                if (_buildConfig != value)
                {
                    _buildConfig = value;
                    OnPropertyChanged(nameof(BuildConfig));
                }
            }
        }

        public BuildConfiguration StandAloneBuildConfig => BuildConfig == 0 ? BuildConfiguration.Debug : BuildConfiguration.Release;
        public BuildConfiguration DLLBuildConfig => BuildConfig == 0 ? BuildConfiguration.DebugEditor : BuildConfiguration.ReleaseEditor;

        private string[] _availableScripts;
        public string[] AvailableScripts{
            get => _availableScripts;
            set
            {
                if(_availableScripts != value)
                {
                    _availableScripts = value;
                    OnPropertyChanged(nameof(AvailableScripts));
                }
            }
        }


        [DataMember(Name = "Scenes")]
        private ObservableCollection<Scene> _scenes = new ObservableCollection<Scene>();
        public ReadOnlyObservableCollection<Scene> Scenes { get; private set; }

        public static UndoRedo UndoRedoManager { get; } = new UndoRedo();

        public ICommand RemoveScene { get; set; }
        public ICommand AddScene { get; set; }
        public ICommand UndoCommand { get; set; }
        public ICommand RedoCommand { get; set; }
        public ICommand SaveCommand { get; set; }
        public ICommand BuildCommand { get; private set; }

        private void SetCommands()
        {
            AddScene = new RelayCommand<object>(x =>
            {
                AddSceneInternal($"New Scene {_scenes.Count}");
                var newScene = _scenes.Last();
                var sceneIndex = _scenes.Count - 1;
                UndoRedoManager.Add(new UndoRedoAction(
                    () => RemoveSceneInternal(newScene),
                    () => _scenes.Insert(sceneIndex, newScene),
                    $"Add {newScene.Name}"
                    ));
            });

            RemoveScene = new RelayCommand<Scene>(x =>
            {
                var sceneIndex = _scenes.IndexOf(x);
                RemoveSceneInternal(x);

                UndoRedoManager.Add(new UndoRedoAction(
                    () => _scenes.Insert(sceneIndex, x),
                    () => RemoveSceneInternal(x),
                    $"Remove {x.Name}"
                    ));
            },
            x => !x.IsActive
            );

            UndoCommand = new RelayCommand<object>(x => UndoRedoManager.Undo(), x => UndoRedoManager.UndoList.Any());
            RedoCommand = new RelayCommand<object>(x => UndoRedoManager.Redo(), x => UndoRedoManager.RedoList.Any());
            BuildCommand = new RelayCommand<bool>(async x => await BuildGameCodeDLL(x), x => !VisualStudio.IsDebugging() && VisualStudio.BuildDone);
            SaveCommand = new RelayCommand<object>(x => Save(this));

            OnPropertyChanged(nameof(AddScene));
            OnPropertyChanged(nameof(RemoveScene));
            OnPropertyChanged(nameof(UndoCommand));
            OnPropertyChanged(nameof(RedoCommand));
            OnPropertyChanged(nameof(BuildCommand));
            OnPropertyChanged(nameof(SaveCommand));
        }

        private static string GetConfigurationName(BuildConfiguration config) => _buildConfigurationNames[(int)config];

        private void AddSceneInternal(in string sceneName){
            Debug.Assert(!string.IsNullOrEmpty(sceneName.Trim()));
            _scenes.Add(new Scene(this, sceneName));
        }
        private void RemoveSceneInternal(in Scene scene)
        {
            Debug.Assert(scene != null);
            Debug.Assert(_scenes.Contains(scene));
            _scenes.Remove(scene);
        }

        public static Project CurrentProject => Application.Current.MainWindow?.DataContext as Project;

        private Scene _activeScene;
        [DataMember]
        public Scene ActiveScene
        {
            get { 
                return _activeScene; 
            }
            set {
                if (_activeScene != value)
                {
                    _activeScene = value;
                    OnPropertyChanged(nameof(ActiveScene));
                }
            }
        }

        public static Project Load(string file)
        {
            Debug.Assert(File.Exists(file));
            return Serializer.FromFile<Project>(file);
        }

        public static void Save(Project projectToSave)
        {
            Serializer.ToFile(projectToSave, projectToSave.FullPath);
            Logger.Log(MessageType.Info, $"Project saved to {projectToSave.FullPath}");
        }

        private async Task BuildGameCodeDLL(bool showWindow = true)
        {
            try
            {
                UnloadGameCodeDLL();
                await Task.Run(() => VisualStudio.BuildSolution(this, GetConfigurationName(DLLBuildConfig), showWindow));
                if (VisualStudio.BuildSucceeded)
                {
                    LoadGameCodeDLL();
                }
            }
            catch (Exception e) {
                Debug.WriteLine(e.Message);
                throw;
            }
        }

        private void LoadGameCodeDLL()
        {
            var configName = GetConfigurationName(DLLBuildConfig);
            var dll = $@"{Path}x64\{configName}\{Name}.dll";
            AvailableScripts = null;
            if(File.Exists(dll) && EngineAPI.LoadGameCodeDLL(dll) != 0)
            {
                AvailableScripts = EngineAPI.EngineDLL_GetScriptNames();
                ActiveScene.GameEntities.Where(x => x.GetComponent<Сomponents.ScriptComponent>() != null).ToList().ForEach(x => x.IsActive = true);
                Logger.Log(MessageType.Info, "The game code DLL loaded successfully");
            }
            else
            {
                Logger.Log(MessageType.Warning, "The game code DLL failed to load. Try to build the project first.");
            }
        }

        private void UnloadGameCodeDLL()
        {
            ActiveScene.GameEntities.Where(x => x.GetComponent<Сomponents.ScriptComponent>() != null).ToList().ForEach(x => x.IsActive = false);
            if (EngineAPI.UnloadGameCodeDLL() != 0)
            {
                Logger.Log(MessageType.Info, "The game code DLL unloaded successfully.");
                AvailableScripts = null;
            }
        }

        public void Unload()
        {
            UnloadGameCodeDLL();
            VisualStudio.CloseVisualStudio();
            UndoRedoManager.Reset();
        }

        [OnDeserialized]
        private async void OnDeserialized(StreamingContext streamingContext)
        {
            if(_scenes != null)
            {
                Scenes = new ReadOnlyObservableCollection<Scene>(_scenes);
                OnPropertyChanged(nameof(Scenes));
            }
            ActiveScene = Scenes.FirstOrDefault(x => x.IsActive);
            Debug.Assert(ActiveScene != null);

            await BuildGameCodeDLL(false);

            SetCommands();
        }

        public Project(string name, string path)
        {
            Name = name;
            Path = path;

            _scenes.Add(new Scene(this, "Default Scene") { IsActive = true });
            OnDeserialized(new StreamingContext());
        }

        public Project(){

        }

    }
}
