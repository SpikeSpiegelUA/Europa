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
        private static readonly DelayEventTimer refreshTimer = new DelayEventTimer(TimeSpan.FromMilliseconds(250));
        private static readonly Dictionary<string, AssetInfo> assetDictionary = new Dictionary<string, AssetInfo>();
        private static readonly ObservableCollection<AssetInfo> assets = new ObservableCollection<AssetInfo>();
        private static readonly FileSystemWatcher contentWatcher = new FileSystemWatcher()
        {
            IncludeSubdirectories = true,
            Filter = "",
            NotifyFilter = NotifyFilters.CreationTime | NotifyFilters.DirectoryName | NotifyFilters.FileName | NotifyFilters.LastWrite
        };

        static AssetRegistry()
        {
            contentWatcher.Changed += OnContentModified;
            contentWatcher.Created += OnContentModified;
            contentWatcher.Deleted += OnContentModified;
            contentWatcher.Renamed += OnContentModified;

            refreshTimer.Triggered += Refresh;
        }

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

        private static async void OnContentModified(object sender, FileSystemEventArgs e)
        {
            var x = 0;
            if (Path.GetExtension(e.FullPath) != Asset.AssetFileExtension)
                return;

            await Application.Current.Dispatcher.BeginInvoke(new Action(() =>
            {
                refreshTimer.Trigger(e);
            }));
        }

        private static void Refresh(object? sender, DelayEventTimerArgs e)
        {
            foreach(var item in e.Data)
            {
                if (!(item is FileSystemEventArgs eventArgs))
                    continue;

                if (eventArgs.ChangeType == WatcherChangeTypes.Deleted)
                {
                    UnregisterAsset(eventArgs.FullPath);
                }
                else
                {
                    RegisterAsset(eventArgs.FullPath);
                    if(eventArgs.ChangeType == WatcherChangeTypes.Renamed)
                    {
                        assetDictionary.Keys.Where(key => !File.Exists(key)).ToList().ForEach(file => UnregisterAsset(file));                   
                    }
                }

            }
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

        public static void Clear()
        {
            contentWatcher.EnableRaisingEvents = false;
            assetDictionary.Clear();
            assets.Clear();
        }

        public static AssetInfo GetAssetInfo(string file) => assetDictionary.ContainsKey(file) ? assetDictionary[file] : null;
        public static AssetInfo GetAssetInfo(Guid guid) => assets.FirstOrDefault(x => x.Guid == guid);

        public static void Reset(string contentFolder)
        {
            Clear();
            Debug.Assert(Directory.Exists(contentFolder));
            RegisterAllAssets(contentFolder);
            contentWatcher.Path = contentFolder;
            contentWatcher.EnableRaisingEvents = true;
        }
    }
}
