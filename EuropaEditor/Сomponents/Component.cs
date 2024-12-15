using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;

namespace EuropaEditor.Сomponents
{
    [DataContract]
    public class Component : ViewModelBase
    {
        [DataMember]
        public GameEntity Owner;

        public Component(GameEntity owner)
        {
            Debug.Assert(owner != null);
            Owner = owner;
        }
    }
}
