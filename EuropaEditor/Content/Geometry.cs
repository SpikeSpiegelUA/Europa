using EuropaEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EuropaEditor.Content
{
    enum PrimitiveMeshType
    {
        Plane,
        Cube,
        UVSphere,
        IcoSphere,
        Cylinder,
        Capsule
    }

    class Mesh : ViewModelBase
    {
        private int _vertexSize;
        public int VertexSize
        {
            get
            {
                return _vertexSize;
            }
            set
            {
                if (_vertexSize != value)
                {
                    _vertexSize = value;
                    OnPropertyChanged(nameof(VertexSize));
                }
            }
        }

        private int _vertexCount;
        public int VertexCount
        {
            get
            {
                return _vertexCount;
            }
            set
            {
                if (_vertexCount != value)
                {
                    _vertexCount = value;
                    OnPropertyChanged(nameof(VertexCount));
                }
            }
        }

        private int _indexSize;
        public int IndexSize
        {
            get
            {
                return _indexSize;
            }
            set
            {
                if (_indexSize != value)
                {
                    _indexSize = value;
                    OnPropertyChanged(nameof(IndexSize));
                }
            }
        }

        private int _indexCount;
        public int IndexCount
        {
            get
            {
                return _indexCount;
            }
            set
            {
                if (_indexCount != value)
                {
                    _indexCount = value;
                    OnPropertyChanged(nameof(IndexCount));
                }
            }
        }

        public byte[] Vertices { get; set; }
        public byte[] Indices { get; set; }

    }

    class MeshLOD : ViewModelBase
    {
        private string _name;

        public string Name
        {
            get => _name;
            set
            {
                if(_name != value)
                {
                    _name = value;
                    OnPropertyChanged("Name");
                }
            }
        }

        private float _lodThreshold;
        public float LODThreshold
        {
            get => _lodThreshold;
            set
            {
                if (value != _lodThreshold)
                {
                    _lodThreshold = value;
                    OnPropertyChanged(nameof(LODThreshold));
                }
            }
        }

        public ObservableCollection<Mesh> Meshes { get; } = new ObservableCollection<Mesh>();
    }

    class LODGroup : ViewModelBase
    {
        private string _name;
        public string Name
        {
            get => _name;
            set
            {
                if(_name != value)
                {
                    _name = value;
                    OnPropertyChanged("Name");
                }
            }
        }

        public ObservableCollection<MeshLOD> LODs { get; } = new ObservableCollection<MeshLOD>();
    }

    class Geometry : Asset
    {

        private readonly List<LODGroup> _lodGroups = new List<LODGroup>();

        public LODGroup GetLODGroup(int lodGroup = 0)
        {
            Debug.Assert(lodGroup >= 0 && lodGroup < _lodGroups.Count);
            return _lodGroups.Any() ? _lodGroups[lodGroup] : null;
        }

        public Geometry() : base(AssetType.Mesh)
        {

        }

        internal void FromRawData(byte[] data)
        {
            Debug.Assert(data.Length > 0);
            _lodGroups.Clear();

            using var reader = new BinaryReader(new MemoryStream(data));

            //Skip scene name string.
            var s = reader.ReadInt32();
            reader.BaseStream.Position += s;
            //Get number of LODs.
            var numberOfLODGroups = reader.ReadInt32();
            Debug.Assert(numberOfLODGroups > 0);

            for (int i = 0; i < numberOfLODGroups; ++i)
            {
                //Get LOD group's name
                s = reader.ReadInt32();
                string lodGroupName;
                if(s > 0)
                {
                    var nameBytes = reader.ReadBytes(s);
                    lodGroupName = Encoding.UTF8.GetString(nameBytes);

                }
                else {
                    lodGroupName = $"lod_{ContentHelper.GetRandomString()}";
                }

                //Get number of meshes in this LOD group
                var numMeshes = reader.ReadInt32();
                Debug.Assert(numMeshes > 0);
                List<MeshLOD> lods = ReadMeshLODs(numMeshes, reader);

                var lodGroup = new LODGroup() { Name = lodGroupName };
                lods.ForEach(l => lodGroup.LODs.Add(l));

                _lodGroups.Add(lodGroup);   
            }
        }

        private List<MeshLOD> ReadMeshLODs(int numMeshes, BinaryReader reader)
        {
            var lodIDs = new List<int>();
            var lodList = new List<MeshLOD>();
            for(int i = 0; i < numMeshes; ++i)
            {
                ReadMeshes(reader, lodIDs, lodList);
            }
            return lodList;
        }

        private void ReadMeshes(BinaryReader reader, List<int> lodIDs, List<MeshLOD> lodList)
        {
            //Get mesh's name
            var s = reader.ReadInt32();
            string meshName;
            if (s > 0)
            {
                var nameBytes = reader.ReadBytes(s);
                meshName = Encoding.UTF8.GetString(nameBytes);

            }
            else
            {
                meshName = $"lod_{ContentHelper.GetRandomString()}";
            }

            var mesh = new Mesh();

            var lodID = reader.ReadInt32();
            mesh.VertexSize = reader.ReadInt32();
            mesh.VertexCount = reader.ReadInt32();
            mesh.IndexSize = reader.ReadInt32();
            mesh.IndexCount = reader.ReadInt32();
            var lodThreshold = reader.ReadSingle();

            var vertexBufferSize = mesh.VertexSize * mesh.VertexCount;
            var indexBufferSize = mesh.IndexSize * mesh.IndexCount;

            mesh.Vertices = reader.ReadBytes(vertexBufferSize);
            mesh.Indices = reader.ReadBytes(indexBufferSize);

            MeshLOD lod;
            if(ID.IsValid(lodID) && lodIDs.Contains(lodID))
            {
                lod = lodList[lodIDs.IndexOf(lodID)];
                Debug.Assert(lod != null);
            }
            else
            {
                lodIDs.Add(lodID);
                lod = new MeshLOD() { Name = meshName, LODThreshold = lodThreshold};
                lodList.Add(lod);
            }

            lod.Meshes.Add(mesh);
        }
    }
}
