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
            IconSmall = smallIcon ?? icon;
            FullPath = fullPath;
        }
    }
    internal class ContentBrowser : ViewModelBase, IDisposable
    {
        private readonly DelayEventTimer refreshTimer = new DelayEventTimer(TimeSpan.FromMicroseconds(250));
       
        public ContentBrowser(Project project)
        {
            Debug.Assert(project != null);
            var contentFolder = project.ContentPath;
            Debug.Assert(!string.IsNullOrEmpty(contentFolder.Trim()));
            contentFolder = Path.TrimEndingDirectorySeparator(contentFolder);
            ContentFolder = contentFolder;
            SelectedFolder = contentFolder;
            FolderContent = new ReadOnlyObservableCollection<ContentInfo>(folderContent);

            ContentWatcher.ContentModified += OnContentModified;
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
                        _ = GetFolderContent();
                    }
                    OnPropertyChanged(nameof(SelectedFolder));
                }
            }
        }
        private void Refresh(object? sender, DelayEventTimerArgs e)
        {
            _ = GetFolderContent();
        }

        private async Task GetFolderContent()
        {
            var folderContent = new List<ContentInfo>();
            await Task.Run(() =>
            {
                folderContent = GetFolderContent(SelectedFolder);
            });

            folderContent.Clear();
            folderContent.ForEach(x => folderContent.Add(x));
        }

        private static List<ContentInfo> GetFolderContent(string path)
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
                foreach (var file in Directory.GetFiles(path, $"*{Asset.AssetFileExtension}"))
                {
                    var fileInfo = new FileInfo(file);

                    folderContent.Add(ContentInfoCache.Add(file));
                }
            }
            catch(IOException ex)
            {
                Debug.WriteLine(ex.Message);
            }

            return folderContent;
        }

        private void OnContentModified(object sender, ContentModifiedEventArgs e)
        {
            if (Path.GetDirectoryName(e.FullPath) != SelectedFolder)
                return;

            refreshTimer.Trigger();
        }

        public void Dispose()
        {
            ContentWatcher.ContentModified += OnContentModified;
            ContentInfoCache.Save();
        }
    }
}
