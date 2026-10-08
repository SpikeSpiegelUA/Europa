using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace EuropaEditor.Content
{
    /// <summary>
    /// Interaction logic for SaveDialogue.xaml
    /// </summary>
    public partial class SaveDialogue : Window
    {
        public string SaveFilePath { get; private set; }
        public SaveDialogue()
        {
            InitializeComponent();
            Closing += OnSaveDialogueClosing;
        }

        private void OnSaveDialogueClosing(object? sender, CancelEventArgs e)
        {
            ContentBrowserView.Dispose();
        }

        private void OnSave_Button_Click(object sender, RoutedEventArgs e)
        {
            if(ValidateFileName(out var saveFilePath))
            {
                SaveFilePath = saveFilePath;
                DialogResult = true;
                Close();
            }
        }

        private bool ValidateFileName(out string saveFilePath)
        {
            var contentBrowser = ContentBrowserView.DataContext as ContentBrowser;
            var path = contentBrowser.SelectedFolder;
            if (!Path.EndsInDirectorySeparator(path))
                path += @"\";
            var fileName = fileNameTextBox.Text.Trim();
            if (string.IsNullOrEmpty(fileName))
            {
                saveFilePath = string.Empty;
                return false;
            }

            if (!fileName.EndsWith(Asset.AssetFileExtension))
                fileName += Asset.AssetFileExtension;

            path += $@"{fileName}";
            var isValid = false;
            string errorMsg = string.Empty;

            if(fileName.IndexOfAny(Path.GetInvalidFileNameChars()) != -1)
            {
                errorMsg = "Invalid character(s) used in the asset file name.";
            }
            else if(File.Exists(path) && MessageBox.Show("File Already exists. Overwrite?", "Overwrite file",
                MessageBoxButton.YesNo, MessageBoxImage.Question) == MessageBoxResult.No)
            {
                //Do nothing. Just return false;
            }
            else
            {
                isValid = true;
            }

            if (!string.IsNullOrEmpty(errorMsg))
            {
                MessageBox.Show(errorMsg, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }

            saveFilePath = path;
            return isValid;
        }

        private void OnContentBrowserView_Mouse_DoubleClick(object sender, MouseButtonEventArgs e)
        {
            if((e.OriginalSource as FrameworkElement).DataContext == ContentBrowserView.SelectedItem &&
                ContentBrowserView.SelectedItem.FileName == fileNameTextBox.Text)
            {
                OnSave_Button_Click(sender, null);
            }
        }
    }
}
