using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;

namespace EuropaEditor.Сomponents
{
    [DataContract]
    public class TransformComponent : Component
    {
        private Vector3 _position;
        [DataMember]
        public Vector3 Position
        {
            get { return _position; }
            set 
            {
                if (_position != value)
                {
                    _position = value;
                    OnPropertyChanged(nameof(Position));
                }
            }
        }
        private Vector3 _rotation;
        [DataMember]
        public Vector3 Rotation
        {
            get { return _rotation; }
            set
            {
                if (_rotation != value)
                {
                    _rotation = value;
                    OnPropertyChanged(nameof(Rotation));
                }
            }
        }
        private Vector3 _scale;
        [DataMember]
        public Vector3 Scale
        {
            get { return _scale; }
            set
            {
                if (_scale != value)
                {
                    _scale = value;
                    OnPropertyChanged(nameof(Scale));
                }
            }
        }

        public TransformComponent(GameEntity owner) : base(owner)
        {

        }
    }
}
