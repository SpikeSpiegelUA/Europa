using EuropaEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Text;
using System.Windows;
using System.Windows.Media;

namespace EuropaEditor.Content
{
    static class AssetRegistry
    {
        private static readonly Dictionary<string, AssetInfo> assetDictionary = new Dictionary<string, AssetInfo>();
        private static readonly ObservableCollection<AssetInfo> assets = new ObservableCollection<AssetInfo>();

        public static ReadOnlyObservableCollection<AssetInfo> Assets { get; } = new ReadOnlyObservableCollection<AssetInfo>(assets);

        private static void RegisterAllAssets(string path)
        {
            Debug.Assert(Directory.Exists(path));
            foreach (var entry in Directory.GetFileSystemEntries(path))
            {
                if (ContentHelper.IsDirectory(entry))
                {
                    RegisterAllAssets(entry);
                }
                else
                {
                    RegisterAsset(entry);
                }
            }
        }

        private static void UnregisterAsset(string file)
        {
            if (assetDictionary.ContainsKey(file))
            {
                assets.Remove(assetDictionary[file]);
                assetDictionary.Remove(file);
            }
        }

        private static void OnContentModified(object sender, ContentModifiedEventArgs e)
        {
            if (ContentHelper.IsDirectory(e.FullPath))
            {
                RegisterAllAssets(e.FullPath);
            }
            else if (File.Exists(e.FullPath))
            {
                RegisterAsset(e.FullPath);
            }

            assets.Where(x => !File.Exists(x.FullPath)).ToList().ForEach(x => UnregisterAsset(x.FullPath));
        }

        private static void RegisterAsset(string file)
        {
            Debug.Assert(File.Exists(file));
            try
            {
                var fileInfo = new FileInfo(file);

                if(!assetDictionary.ContainsKey(file) || assetDictionary[file].RegisterTime.IsOlder(fileInfo.LastWriteTime))
                {
                    var info = Asset.GetAssetInfo(file);
                    Debug.Assert(info != null);
                    info.RegisterTime = DateTime.Now;
                    assetDictionary[file] = info;
                    Debug.Assert(assetDictionary.ContainsKey(file));
                    assets.Add(assetDictionary[file]);
                }
            }
            catch(Exception ex)
            {
                Debug.WriteLine(ex.Message);
            }
        }

        public static AssetInfo GetAssetInfo(string file) => assetDictionary.ContainsKey(file) ? assetDictionary[file] : null;
        public static AssetInfo GetAssetInfo(Guid guid) => assets.FirstOrDefault(x => x.Guid == guid);

        public static void Reset(string contentFolder)
        {
            ContentWatcher.ContentModified -= OnContentModified;

            assetDictionary.Clear();
            assets.Clear();

            Debug.Assert(Directory.Exists(contentFolder));
            RegisterAllAssets(contentFolder);

            ContentWatcher.ContentModified += OnContentModified;
        }
    }
}
