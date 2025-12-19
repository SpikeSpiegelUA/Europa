using EuropaEditor.Content;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media.Media3D;

namespace EuropaEditor.Editors
{

    class MeshRendererVertexData : ViewModelBase
    {
        public Point3DCollection Positions { get; } = new Point3DCollection();
        public Vector3DCollection Normals { get; } = new Vector3DCollection();
    }

     class GeometryEditor : ViewModelBase, IAssetEditor
    {
        public Asset Asset => Geometry;

        private Geometry _geometry;

        public Geometry Geometry
        {
            get => _geometry;
            set
            {
                if (_geometry != value)
                {
                    _geometry = value;
                    OnPropertyChanged(nameof(Geometry));
                }
            }
        }

        public void SetAsset(Asset asset)
        {
            Debug.Assert(asset is Content.Geometry);
            if (asset is Content.Geometry geometry)
            {
                Geometry = geometry;
            }
        }
    }
}
