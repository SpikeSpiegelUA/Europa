using EuropaEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Text;

namespace EuropaEditor.Content
{
    static class ContentInfoCache
    {
        private static readonly object lockObject = new object();
        private static string cacheFilePath = string.Empty;
        private static bool isDirty;
        private static readonly Dictionary<string, ContentInfo> contentInfoCache = new Dictionary<string, ContentInfo>();

        public static ContentInfo Add(string file)
        {
            lock (lockObject)
            {
                var fileInfo = new FileInfo(file);
                Debug.Assert(!fileInfo.IsDirectory());

                if (!contentInfoCache.ContainsKey(file) || contentInfoCache[file].DateModified.IsOlder(fileInfo.LastWriteTime))
                {
                    var info = AssetRegistry.GetAssetInfo(file) ?? Asset.GetAssetInfo(file);
                    Debug.Assert(info != null);
                    contentInfoCache[file] = new ContentInfo(file, info.Icon);
                    isDirty = true;
                }

                Debug.Assert(contentInfoCache.ContainsKey(file));
                return contentInfoCache[file];
            }
        }

        public static void Reset(string projectPath)
        {
            lock (lockObject)
            {
                if(!string.IsNullOrEmpty(cacheFilePath) && isDirty)
                {
                    SaveInfoCache();
                    cacheFilePath = string.Empty;
                    contentInfoCache.Clear();
                    isDirty = false;
                }

                if (!string.IsNullOrEmpty(projectPath))
                {
                    Debug.Assert(Directory.Exists(projectPath));
                    cacheFilePath = $@"{projectPath}.Europa\ContentInfoCache.bin";
                    LoadInfoCache();
                }
            }
        }

        private static void SaveInfoCache()
        {
            try
            {
                using var writer = new BinaryWriter(File.Open(cacheFilePath, FileMode.Create, FileAccess.Write));
                writer.Write(contentInfoCache.Keys.Count);
                foreach (var key in contentInfoCache.Keys)
                {
                    var info = contentInfoCache[key];

                    writer.Write(key);
                    writer.Write(info.DateModified.ToBinary());
                    writer.Write(info.Icon.Length);
                    writer.Write(info.Icon);
                }
                isDirty = false;
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.Log(MessageType.Warning, "Failed to save Content Browser cache file.");
            }
        }

        public static void Save() => Reset(string.Empty);

        private static void LoadInfoCache()
        {
            if (!File.Exists(cacheFilePath))
                return;

            try
            {
                using var reader = new BinaryReader(File.Open(cacheFilePath, FileMode.Open, FileAccess.Read));
                var numEntries = reader.ReadInt32();
                contentInfoCache.Clear();

                for (int i = 0; i < numEntries; ++i)
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
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.Log(MessageType.Warning, "Failed to read Content Browser cache file.");
                contentInfoCache.Clear();
            }
        }
    }
}
