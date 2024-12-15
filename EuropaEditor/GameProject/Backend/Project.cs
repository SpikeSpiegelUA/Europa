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
    public class Project : ViewModelBase
    {
        public static string Extension = ".europa";
        [DataMember]
        public string Name { get; private set; } = "New Project";
        [DataMember]
        public string Path { get; private set; }
        public string FullPath => $@"{Path}{Name}\{Name}{Extension}";

        [DataMember(Name = "Scenes")]
        private ObservableCollection<Scene> _scenes = new ObservableCollection<Scene>();
        public ReadOnlyObservableCollection<Scene> Scenes { get; private set; }

        public static UndoRedo UndoRedoManager { get; } = new UndoRedo();

        public ICommand RemoveScene { get; set; }
        public ICommand AddScene { get; set; }
        public ICommand UndoCommand { get; set; }
        public ICommand RedoCommand { get; set; }
        public ICommand SaveCommand { get; set; }

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
        }

        public void Unload()
        {

        }

        [OnDeserialized]
        private void OnDeserialized(StreamingContext streamingContext)
        {
            if(_scenes != null)
            {
                Scenes = new ReadOnlyObservableCollection<Scene>(_scenes);
                OnPropertyChanged(nameof(Scenes));
            }
            ActiveScene = Scenes.FirstOrDefault(x => x.IsActive);
            AddScene = new RelayCommand<object>(x =>
            {
                AddSceneInternal($"New Scene {_scenes.Count}");
                var newScene = _scenes.Last();
                var sceneIndex = _scenes.Count - 1;
                UndoRedoManager.AddUndo(new UndoRedoAction(
                    () => RemoveSceneInternal(newScene),
                    () => _scenes.Insert(sceneIndex, newScene),
                    $"Add {newScene.Name}"
                    ));
            });

            RemoveScene = new RelayCommand<Scene>(x =>
            {
                var sceneIndex = _scenes.IndexOf(x);
                RemoveSceneInternal(x);

                UndoRedoManager.AddUndo(new UndoRedoAction(
                    () => _scenes.Insert(sceneIndex, x),
                    () => RemoveSceneInternal(x),
                    $"Remove {x.Name}"
                    ));
            }, 
            x => !x.IsActive
            );

            UndoCommand = new RelayCommand<object>(x => UndoRedoManager.Undo(), x => UndoRedoManager.UndoList.Any());
            RedoCommand = new RelayCommand<object>(x => UndoRedoManager.Redo(), x => UndoRedoManager.RedoList.Any());

            SaveCommand = new RelayCommand<object>(x => Save(this));
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
