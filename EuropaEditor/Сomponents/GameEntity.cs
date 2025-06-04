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
using EuropaEditor.DLLWrapper;
using System.Xml.Linq;

namespace EuropaEditor.Сomponents
{
    [DataContract]
    [KnownType(typeof(TransformComponent))]
    [KnownType(typeof(ScriptComponent))]
    class GameEntity : ViewModelBase
    {
        private int _entityID = ID.INVALID_ID;
        public int EntityID
        {
            get => _entityID;
            set
            {
                if (_entityID != value)
                {
                    _entityID = value;
                    OnPropertyChanged(nameof(EntityID));
                }
            }
        }

        private bool _isActive;
        public bool IsActive {
            get => _isActive;
            set
            {
                if (_isActive != value) {
                    _isActive = value;
                    if (_isActive)
                    {
                        EntityID = EngineAPI.EntityAPI.CreateGameEntity(this);
                        Debug.Assert(ID.IsValid(_entityID));
                    }
                    else if(ID.IsValid(EntityID))
                    {
                        EngineAPI.EntityAPI.RemoveGameEntity(this);
                        EntityID = ID.INVALID_ID;
                    }
                    OnPropertyChanged(nameof(IsActive));
                }
            } 
        }

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

        public Component GetComponent(Type type) => Components.FirstOrDefault(c => c.GetType() == type);
        public T GetComponent<T>() where T : Component => GetComponent(typeof(T)) as T;

        public bool AddComponent(Component component)
        {
            Debug.Assert(component != null);
            if (!Components.Any(x => x.GetType() == component.GetType()))
            {
                IsActive = false;
                _components.Add(component);
                IsActive = true;
                return true;
            }
            else
            {
                Logger.Log(MessageType.Warning, $"Entity {Name} already has a {component.GetType().Name} component");
                return false;
            }
        }

        public void RemoveComponent(Component component)
        {
            Debug.Assert(component != null);
            //Transform component can't be removed.
            if (component is TransformComponent)
                return;
            if (_components.Contains(component))
            {
                IsActive = false;
                _components.Remove(component);
                IsActive = true;
            }
        }

        [DataMember]
        public Scene ParentScene { get; set; }

        [OnDeserialized]
        void OnDeserialized(StreamingContext context)
        {
            Components = new ReadOnlyObservableCollection<Component>(_components);
            OnPropertyChanged(nameof(Components));
        }

        public GameEntity()
        {

        }

        internal GameEntity(Scene parentScene)
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
        //Control if this class can update its "children" entities.
        private bool _enableUpdates = true;
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

        public T GetMSComponent<T>() where T : IMSComponent
        {
            return (T)Components.FirstOrDefault(x => x.GetType() == typeof(T));
        }

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

        //We use this function to create a list of components, that all the entities have.
        public void MakeComponentList()
        {
            _components.Clear();
            var firstEntity = SelectedEntities.FirstOrDefault();
            if (firstEntity == null)
                return;

            foreach (var component in firstEntity.Components)
            {
                var type = component.GetType();
                if(!SelectedEntities.Skip(1).Any(entity => entity.GetComponent(type) == null))
                {
                    Debug.Assert(Components.FirstOrDefault(x => x.GetType() == type) == null);
                    _components.Add(component.GetMultiselectionComponent(this));
                }

            }
        }

        public static float? GetMixedValue<T>(List<T> objects, Func<T, float> getProperty)
        {
            var value = getProperty(objects.First());
            return objects.Skip(1).Any(c => !value.IsTheSameAs(getProperty(c))) ? (float?)null : value;
        }

        public static bool? GetMixedValue<T>(List<T> objects, Func<T, bool?> getProperty)
        {
            var value = getProperty(objects.First());
            return objects.Skip(1).Any(c => value != getProperty(c)) ? (bool?)null : value;
        }

        public static string GetMixedValue<T>(List<T> objects, Func<T, string> getProperty)
        {
            var value = getProperty(objects.First());
            return objects.Skip(1).Any(c => value != getProperty(c)) ? null : value;
        }

        protected virtual bool UpdateMSGameEntity()
        {
            IsEnabled = GetMixedValue(SelectedEntities, new Func<GameEntity, bool?>(x => x.IsEnabled));
            Name = GetMixedValue(SelectedEntities, new Func<GameEntity, string>(x => x.Name));
            return true;
        }

        public void Refresh()
        {
            _enableUpdates = false;
            UpdateMSGameEntity();
            MakeComponentList();
            _enableUpdates = true;
        }

        public MSEntity(List<GameEntity> selectedEntities)
        {
            Debug.Assert(selectedEntities?.Any() == true);
            Components = new ReadOnlyObservableCollection<IMSComponent>(_components);
            SelectedEntities = selectedEntities;
            PropertyChanged += (s, e) =>
            {
                if(_enableUpdates)
                    UpdateGameEntities(e.PropertyName);
            };
        }
    }

    class MSGameEntity : MSEntity
    {
        public MSGameEntity() : base(null)
        {

        }
        public MSGameEntity(List<GameEntity> entities) : base(entities)
        {
            Refresh();
        }
    }
}
