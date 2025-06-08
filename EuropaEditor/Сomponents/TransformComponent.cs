using EuropaEditor.Utilities;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;

namespace EuropaEditor.Сomponents
{
    [DataContract]
    class TransformComponent : Component
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

        public override IMSComponent GetMultiselectionComponent(MSEntity msEntity) => new MSTransformComponent(msEntity);

        public override void WriteToBinary(BinaryWriter bw)
        {
            bw.Write(Position.X);
            bw.Write(Position.Y);
            bw.Write(Position.Z);
            bw.Write(Rotation.X);
            bw.Write(Rotation.Y);
            bw.Write(Rotation.Z);
            bw.Write(Scale.X);
            bw.Write(Scale.Y);
            bw.Write(Scale.Z);
        }
    }

    sealed class MSTransformComponent : MSComponent<TransformComponent>
    {
        private float? _posX;
        public float? PosX
        {
            get { return _posX; }
            set
            {
                if (!_posX.IsTheSameAs(value))
                {
                    _posX = value;
                    OnPropertyChanged(nameof(PosX));
                }
            }
        }
        private float? _posY;
        public float? PosY
        {
            get { return _posY; }
            set
            {
                if (!_posY.IsTheSameAs(value))
                {
                    _posY = value;
                    OnPropertyChanged(nameof(PosY));
                }
            }
        }
        private float? _posZ;
        public float? PosZ
        {
            get { return _posZ; }
            set
            {
                if (!_posZ.IsTheSameAs(value))
                {
                    _posZ = value;
                    OnPropertyChanged(nameof(PosZ));
                }
            }
        }
        private float? _rotX;
        public float? RotX
        {
            get { return _rotX; }
            set
            {
                if (!_rotX.IsTheSameAs(value))
                {
                    _rotX = value;
                    OnPropertyChanged(nameof(RotX));
                }
            }
        }
        private float? _rotY;
        public float? RotY
        {
            get { return _rotY; }
            set
            {
                if (!_rotY.IsTheSameAs(value))
                {
                    _rotY = value;
                    OnPropertyChanged(nameof(RotY));
                }
            }
        }
        private float? _rotZ;
        public float? RotZ
        {
            get { return _rotZ; }
            set
            {
                if (!_rotZ.IsTheSameAs(value))
                {
                    _rotZ = value;
                    OnPropertyChanged(nameof(RotZ));
                }
            }
        }
        private float? _scaleX;
        public float? ScaleX
        {
            get { return _scaleX; }
            set
            {
                if (!_scaleX.IsTheSameAs(value))
                {
                    _scaleX = value;
                    OnPropertyChanged(nameof(ScaleX));
                }
            }
        }
        private float? _scaleY;
        public float? ScaleY
        {
            get { return _scaleY; }
            set
            {
                if (!_scaleY.IsTheSameAs(value))
                {
                    _scaleY = value;
                    OnPropertyChanged(nameof(ScaleY));
                }
            }
        }
        private float? _scaleZ;
        public float? ScaleZ
        {
            get { return _scaleZ; }
            set
            {
                if (!_scaleZ.IsTheSameAs(value))
                {
                    _scaleZ = value;
                    OnPropertyChanged(nameof(ScaleZ));
                }
            }
        }
        protected override bool UpdateComponents(string propertyName)
        {
            switch (propertyName)
            {
                case nameof(PosX):
                case nameof(PosY):
                case nameof(PosZ):
                    SelectedComponents.ForEach(c => c.Position = new Vector3(_posX ?? c.Position.X, _posY ?? c.Position.Y, _posZ ?? c.Position.Z));
                    return true;

                case nameof(RotX):
                case nameof(RotY):
                case nameof(RotZ):
                    SelectedComponents.ForEach(c => c.Rotation = new Vector3(_rotX ?? c.Rotation.X, _rotY ?? c.Rotation.Y, _rotZ ?? c.Rotation.Z));
                    return true;

                case nameof(ScaleX):
                case nameof(ScaleY):
                case nameof(ScaleZ):
                    SelectedComponents.ForEach(c => c.Scale = new Vector3(_scaleX ?? c.Scale.X, _scaleY ?? c.Scale.Y, _scaleZ ?? c.Scale.Z));
                    return true;
            }
            return false;
        }

        protected override bool UpdateMSComponent()
        {
            PosX = MSEntity.GetMixedValue(SelectedComponents, new Func<TransformComponent, float>(x => x.Position.X));
            PosY = MSEntity.GetMixedValue(SelectedComponents, new Func<TransformComponent, float>(x => x.Position.Y));
            PosZ = MSEntity.GetMixedValue(SelectedComponents, new Func<TransformComponent, float>(x => x.Position.Z));

            RotX = MSEntity.GetMixedValue(SelectedComponents, new Func<TransformComponent, float>(x => x.Rotation.X));
            RotY = MSEntity.GetMixedValue(SelectedComponents, new Func<TransformComponent, float>(x => x.Rotation.Y));
            RotZ = MSEntity.GetMixedValue(SelectedComponents, new Func<TransformComponent, float>(x => x.Rotation.Z));

            ScaleX = MSEntity.GetMixedValue(SelectedComponents, new Func<TransformComponent, float>(x => x.Scale.X));
            ScaleY = MSEntity.GetMixedValue(SelectedComponents, new Func<TransformComponent, float>(x => x.Scale.Y));
            ScaleZ = MSEntity.GetMixedValue(SelectedComponents, new Func<TransformComponent, float>(x => x.Scale.Z));

            return true;
        }

        public MSTransformComponent(MSEntity msEntity) : base(msEntity)
        {
            Refresh();
        }
    }
}
