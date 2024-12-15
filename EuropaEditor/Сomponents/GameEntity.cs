using EuropaEditor.GameProject.Backend;
using EuropaEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;

namespace EuropaEditor.Сomponents
{
    [DataContract]
    [KnownType(typeof(TransformComponent))]
    public class GameEntity : ViewModelBase
    {
        private bool _isEnabled = true;
        [DataMember]
        public bool IsEnabled
        {
            get
            {
                return _isEnabled;
            }
            set
            {
                if (_isEnabled != value)
                {
                    _isEnabled = value;
                    OnPropertyChanged(nameof(IsEnabled));
                }
            }
        }
        private string _name;
        [DataMember]
        public string Name
        {
            get { return _name; }
            set
            {
                if (_name != value)
                {
                    _name = value;
                    OnPropertyChanged(nameof(Name));
                }
            }
        }

        [DataMember(Name = "Components")]
        private ObservableCollection<Component> _components = new ObservableCollection<Component>();
        public ReadOnlyObservableCollection<Component> Components { get; private set; }
        
        public ICommand RenameCommand { get; private set; }
        public ICommand EnableCommand { get; private set; }

        [DataMember]
        public Scene ParentScene { get; set; }

        [OnDeserialized]
        void OnDeserialized(StreamingContext context)
        {
            Components = new ReadOnlyObservableCollection<Component>(_components);
            OnPropertyChanged(nameof(Components));

            RenameCommand = new RelayCommand<string>(x =>
            {
                var oldName = _name;
                Name = x;

                Project.UndoRedoManager.AddUndo(new UndoRedoAction(nameof(Name), this, oldName, x,
                    $"Change the name of a game entity from \"{oldName}\" to \"{x}\""));
            }, x => x != _name);
        }

        public GameEntity()
        {

        }

        public GameEntity(Scene parentScene)
        {
            Debug.Assert(parentScene != null);
            ParentScene = parentScene;
            _components.Add(new TransformComponent(this));
            OnDeserialized(new StreamingContext());
        }
    }
}
