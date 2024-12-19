using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;

namespace EuropaEditor.Сomponents
{
    //Use an empty interface to let _componentns collection in MSEntity have MSComponent as a type.
    //MSComponent is a generic type, so we need to use an interface in that collection.
    interface IMSComponent{

    }

    [DataContract]
    public abstract class Component : ViewModelBase
    {
        [DataMember]
        public GameEntity Owner;

        public Component(GameEntity owner)
        {
            Debug.Assert(owner != null);
            Owner = owner;
        }
    }

    abstract class MSComponent<T> : ViewModelBase, IMSComponent where T : Component
    {

    }
}
