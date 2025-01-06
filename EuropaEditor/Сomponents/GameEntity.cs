﻿using EuropaEditor.GameProject.Backend;
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
    public class GameEntity : ViewModelBase
    {
        private int _entityID = ID.INVALID_ID;
        public int EntityID
        {
            get => _entityID;
            set
            {
                if (_entityID != value)
                {
                    value = _entityID;
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
                        EntityID = EngineAPI.CreateGameEntity(this);
                        Debug.Assert(ID.IsValid(_entityID));
                    }
                    else
                    {
                        EngineAPI.RemoveGameEntity(this);
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
                if (!value.IsTheSameAs(getProperty(entity)))
                    return null;
            }
            return value;
        }

        public static bool? GetMixedValue(List<GameEntity> entities, Func<GameEntity, bool?> getProperty)
        {
            var value = getProperty(entities.First());
            foreach (var entity in entities.Skip(1))
            {
                if (value != getProperty(entity))
                    return null;
            }
            return value;
        }

        public static string GetMixedValue(List<GameEntity> entities, Func<GameEntity, string> getProperty)
        {
            var value = getProperty(entities.First());
            foreach (var entity in entities.Skip(1))
            {
                if (!value.Equals(getProperty(entity)))
                    return null;
            }
            return value;
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
        public MSGameEntity(List<GameEntity> entities) : base(entities)
        {
            Refresh();
        }
    }
}
