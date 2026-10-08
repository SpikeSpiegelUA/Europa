using EuropaEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Text;
using System.Windows;

namespace EuropaEditor.Content
{
    public class ContentModifiedEventArgs : EventArgs
    {
        public string FullPath { get; }

        public ContentModifiedEventArgs(string path)
        {
            FullPath = path;
        }
    }

    static class ContentWatcher
    {
        private static readonly DelayEventTimer refreshTimer = new DelayEventTimer(TimeSpan.FromMilliseconds(250));
        //File watcher is only enabled when this counter is 0.
        private static int fileWatcherEnableCounter = 0;
        private static readonly FileSystemWatcher contentWatcher = new FileSystemWatcher()
        {
            IncludeSubdirectories = true,
            Filter = "",
            NotifyFilter = NotifyFilters.CreationTime | NotifyFilters.DirectoryName | NotifyFilters.FileName | NotifyFilters.LastWrite
        };

        public static event EventHandler<ContentModifiedEventArgs> ContentModified;

        static ContentWatcher()
        {

            contentWatcher.Changed += OnContentModified;
            contentWatcher.Created += OnContentModified;
            contentWatcher.Deleted += OnContentModified;
            contentWatcher.Renamed += OnContentModified;

            refreshTimer.Triggered += Refresh;
        }

        public static void Reset(string contentFolder, string projectPath)
        {
            contentWatcher.EnableRaisingEvents = false;

            ContentInfoCache.Reset(projectPath);

            if (!string.IsNullOrEmpty(contentFolder))
            {
                Debug.Assert(Directory.Exists(contentFolder));
                contentWatcher.Path = contentFolder;
                contentWatcher.EnableRaisingEvents = true;
                AssetRegistry.Reset(contentFolder);
            }
        }

        private static void Refresh(object sender, DelayEventTimerArgs e)
        {
            if(fileWatcherEnableCounter > 0)
            {
                e.RepeatEvent = true;
                return;
            }

            e.Data.Cast<FileSystemEventArgs>().GroupBy(x => x.FullPath).Select(x => x.First()).ToList().
                ForEach(x => ContentModified?.Invoke(null, new ContentModifiedEventArgs(x.FullPath)));
        }

        private static async void OnContentModified(object sender, FileSystemEventArgs e)
        {
            await Application.Current.Dispatcher.BeginInvoke(new Action(() => refreshTimer.Trigger(e)));
        }

        public static void EnableFileWatcher(bool isEnabled)
        {
            if(fileWatcherEnableCounter > 0 && isEnabled)
            {
                --fileWatcherEnableCounter;
            }
            else if (!isEnabled)
            {
                ++fileWatcherEnableCounter;
            }
        }
    }
}
