using EuropaEditor.GameProject.Backend;
using EuropaEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
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
    class GameEntity : ViewModelBase
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
        public ICommand IsEnableCommand { get; private set; }

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

            IsEnableCommand = new RelayCommand<bool>(x =>
            {
                bool oldValue = _isEnabled;
                IsEnabled = x;

                Project.UndoRedoManager.AddUndo(new UndoRedoAction(nameof(IsEnabled), this, oldValue, x,
                    x ? $"Enable {Name}" : $"Disable {Name}"));
            });
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

    //Basic abstract class for multiselection entitites.
    abstract class MSEntity : ViewModelBase
    {
        //Set this to a nullable type, so if values in SelectedEntities are different, set this to null.
        private bool? _isEnabled = true;
        public bool? IsEnabled
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

        //Set type of this to an interface, cause MSComponent is a generic type.
        private readonly ObservableCollection<IMSComponent> _components = new ObservableCollection<IMSComponent>();
        public ReadOnlyObservableCollection<IMSComponent> Components { get; private set; }

        public List<GameEntity> SelectedEntities { get; }

        protected virtual bool UpdateGameEntities(string propertyName)
        {
            switch (propertyName)
            {
                case nameof(Name):
                    SelectedEntities.ForEach(x => x.Name = Name);
                    return true;
                case nameof(IsEnabled):
                    SelectedEntities.ForEach(x => x.IsEnabled = IsEnabled.Value);
                    return true;
            }
            return false;
        }

        public static float? GetMixedValue(List<GameEntity> entities, Func<GameEntity, float> getProperty)
        {
            var value = getProperty(entities.First());
            foreach (var entity in entities.Skip(1))
            {
                if (value != getProperty(entity))
                    return null;
            }
            return value;
        }

        protected virtual bool UpdateMSGameEntity(string propertyName)
        {
            IsEnabled = GetMixedValue(SelectedEntities, new Func<GameEntity, bool>(x => x.IsEnabled));
            IsEnabled = GetMixedValue(SelectedEntities, new Func<GameEntity, string>(x => x.Name));
            return true;
        }

        public void Refrest()
        {
            UpdateMSGameEntity
        }

        public MSEntity(List<GameEntity> selectedEntities)
        {
            Debug.Assert(selectedEntities?.Any() == true);
            Components = new ReadOnlyObservableCollection<IMSComponent>(_components);
            SelectedEntities = selectedEntities;
            PropertyChanged += (s, e) =>
            {
                UpdateGameEntities(e.PropertyName);
            };
        }
    }

    class MSGameEntity : MSEntity
    {

    }
}
