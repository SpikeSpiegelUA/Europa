using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;

namespace EuropaEditor.Сomponents
{
    //Use an empty interface to let _components collection in MSEntity have MSComponent as a type.
    //MSComponent is a generic type, so we need to use an interface in that collection.
    interface IMSComponent{

    }

    [DataContract]
    abstract class Component : ViewModelBase
    {
        [DataMember]
        public GameEntity Owner;
        //Gets the multiselection type of this component.
        public abstract IMSComponent GetMultiselectionComponent(MSEntity msEntity);

        public Component(GameEntity owner)
        {
            Debug.Assert(owner != null);
            Owner = owner;
        }
    }

    abstract class MSComponent<T> : ViewModelBase, IMSComponent where T : Component
    {
        //To prevent circular updates on the PropertyChanged event use this boolean for a flow control.
        private bool _enableUpdates = true;
        public List<T> SelectedComponents { get; }

        //Updates components in the SelectedComponents list.
        protected abstract bool UpdateComponents(string propertyName);

        //Updates the state of this component(or its children).
        protected abstract bool UpdateMSComponent();

        public void Refresh()
        {
            _enableUpdates = false;
            UpdateMSComponent();
            _enableUpdates = true;
        }

        public MSComponent(MSEntity msEntity)
        {
            Debug.Assert(msEntity.SelectedEntities?.Any() == true);
            SelectedComponents = msEntity.SelectedEntities.Select(entity => entity.GetComponent<T>()).ToList();
            PropertyChanged += (s, e) => { if (_enableUpdates) UpdateComponents(e.PropertyName); };
        }
    }
}
