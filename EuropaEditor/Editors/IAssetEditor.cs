using EuropaEditor.Content;

namespace EuropaEditor.Editors
{
    interface IAssetEditor
    {
        Asset Asset { get; }

        void SetAsset(Asset asset);
    }
}