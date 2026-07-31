using EuropaEditor.GameProject.Backend;
using EuropaEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Text;
using System.Windows;

namespace EuropaEditor.Content
{
    sealed class ContentInfo
    {
        public static int IconWidth => 90;
        public byte[] Icon { get; }
        public byte[] IconSmall { get; }
        public string FullPath { get; }
        public string FileName => Path.GetFileNameWithoutExtension(FullPath);
        public bool IsDirectory { get; }
        public DateTime DateModified { get; }
        public long? Size { get; }

        public ContentInfo(string fullPath, byte[] icon = null, byte[] smallIcon = null, DateTime? lastModified = null)
        {
            Debug.Assert(File.Exists(fullPath) || Directory.Exists(fullPath));
            var info = new FileInfo(fullPath);
            IsDirectory = ContentHelper.IsDirectory(fullPath);
            DateModified = lastModified ?? info.LastWriteTime;
            Size = IsDirectory ? (long?)null : info.Length;
            Icon = icon;
            IconSmall = IconSmall ?? icon;
            FullPath = fullPath;
        }
    }
    internal class ContentBrowser : ViewModelBase, IDisposable
    {
        private static readonly object lockObject = new object();
        private static readonly DelayEventTimer refreshTimer = new DelayEventTimer(TimeSpan.FromMilliseconds(250));
        private static readonly FileSystemWatcher contentWatcher = new FileSystemWatcher()
        {
            IncludeSubdirectories = true,
            Filter = "",
            NotifyFilter = NotifyFilters.CreationTime | NotifyFilters.DirectoryName | NotifyFilters.FileName | NotifyFilters.LastWrite
        };
        private static string cacheFilePath = string.Empty;
        private static readonly Dictionary<string, ContentInfo> contentInfoCache = new Dictionary<string, ContentInfo>();
        public ContentBrowser(Project project)
        {
            Debug.Assert(project != null);
            var contentFolder = project.ContentPath;
            Debug.Assert(!string.IsNullOrEmpty(contentFolder.Trim()));
            contentFolder = Path.TrimEndingDirectorySeparator(contentFolder);
            ContentFolder = contentFolder;
            SelectedFolder = contentFolder;
            FolderContent = new ReadOnlyObservableCollection<ContentInfo>(folderContent);

            if (string.IsNullOrEmpty(cacheFilePath))
            {
                cacheFilePath = $@"{project.Path}.Europa\ContentInfoCache.bin";
                LoadInfoCache(cacheFilePath);
            }

            contentWatcher.Path = contentFolder;

            contentWatcher.Changed += OnContentModified;
            contentWatcher.Created += OnContentModified;
            contentWatcher.Deleted += OnContentModified;
            contentWatcher.Renamed += OnContentModified;
            contentWatcher.EnableRaisingEvents = true;

            refreshTimer.Triggered += Refresh;
        }

        public string ContentFolder { get; }
        private readonly ObservableCollection<ContentInfo> folderContent = new ObservableCollection<ContentInfo>();
        public ReadOnlyObservableCollection<ContentInfo> FolderContent { get; }

        private string selectedFolder;
        public string SelectedFolder
        {
            get => selectedFolder;
            set
            {
                if(selectedFolder != value)
                {
                    selectedFolder = value;
                    if (!string.IsNullOrEmpty(selectedFolder))
                    {
                        GetFolderContent();
                    }
                    OnPropertyChanged(nameof(SelectedFolder));
                }
            }
        }
        private void Refresh(object? sender, DelayEventTimerArgs e)
        {
            GetFolderContent();
        }

        private async void GetFolderContent()
        {
            var folderContent = new List<ContentInfo>();
            await Task.Run(() =>
            {
                folderContent = GetFolderContent(SelectedFolder);
            });

            folderContent.Clear();
            folderContent.ForEach(x => folderContent.Add(x));
        }

        private List<ContentInfo> GetFolderContent(string path)
        {
            Debug.Assert(!string.IsNullOrEmpty(path));
            var folderContent = new List<ContentInfo>();

            try
            {
                //Get sub-folder
                foreach(var dir in Directory.GetDirectories(path))
                {
                    folderContent.Add(new ContentInfo(dir));
                }

                //Get files
                lock (lockObject)
                {
                    foreach (var file in Directory.GetFiles(path, $"*{Asset.AssetFileExtension}"))
                    {
                        var fileInfo = new FileInfo(file);

                        if (!contentInfoCache.ContainsKey(file) || contentInfoCache[file].DateModified.IsOlder(fileInfo.LastWriteTime))
                        {
                            var info = AssetRegistry.GetAssetInfo(file) ?? Asset.GetAssetInfo(file);
                            Debug.Assert(info != null);
                            contentInfoCache[file] = new ContentInfo(file, info.Icon);
                        }

                        Debug.Assert(contentInfoCache.ContainsKey(file));
                        folderContent.Add(contentInfoCache[file]);
                    }
                }
            }
            catch(IOException ex)
            {
                Debug.WriteLine(ex.Message);
            }

            return folderContent;
        }

        private async void OnContentModified(object sender, FileSystemEventArgs e)
        {
            if (Path.GetDirectoryName(e.FullPath) != SelectedFolder)
                return;

            await Application.Current.Dispatcher.BeginInvoke(new Action(() =>
            {
                refreshTimer.Trigger();
            }));
        }

        private static void SaveInfoCache(string file)
        {
            lock (lockObject)
            {
                using var writer = new BinaryWriter(File.Open(file, FileMode.Create, FileAccess.Write));
                writer.Write(contentInfoCache.Keys.Count);
                foreach(var key in contentInfoCache.Keys)
                {
                    var info = contentInfoCache[key];

                    writer.Write(key);
                    writer.Write(info.DateModified.ToBinary());
                    writer.Write(info.Icon.Length);
                    writer.Write(info.Icon);
                }
            }
        }

        private static void LoadInfoCache(string file)
        {
            if (!File.Exists(file))
                return;

            try
            {
                lock (lockObject)
                {
                    using var reader = new BinaryReader(File.Open(file, FileMode.Open, FileAccess.Read));
                    var numEntries = reader.ReadInt32();
                    contentInfoCache.Clear();

                    for(int i = 0; i < numEntries; ++i)
                    {
                        var assetFile = reader.ReadString();
                        var date = DateTime.FromBinary(reader.ReadInt64());
                        var iconSize = reader.ReadInt32();
                        var icon = reader.ReadBytes(iconSize);

                        //Cache only the files that still exist.
                        if (File.Exists(assetFile))
                        {
                            contentInfoCache[assetFile] = new ContentInfo(assetFile, icon, null, date);
                        }
                    }
                }
            }
            catch(Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.Log(MessageType.Warning, "Failed to read Content Browser cache file.");
                contentInfoCache.Clear();
            }
        }

        public void Dispose()
        {
            ((IDisposable)contentWatcher).Dispose();
            if (!string.IsNullOrEmpty(cacheFilePath))
            {
                SaveInfoCache(cacheFilePath);
                cacheFilePath = string.Empty;
            }
        }
    }
}
