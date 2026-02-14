using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EuropaEditor.Content
{
    enum AssetType
    {
        Unknown,
        Animation,
        Audio,
        Material,
        Mesh,
        Skeleton,
        Texture
    }

    abstract class Asset : ViewModelBase
    {
        public AssetType Type { get; private set; }
        public byte[] Icon { get; protected set; }
        public string SourcePath { get; protected set; }
        public Guid Guid { get; protected set; } = Guid.NewGuid();
        public abstract IEnumerable<string> Save(string file);

        public Asset(AssetType type)
        {
            Debug.Assert(type != AssetType.Unknown);
            Type = type;
        }
    }
}
