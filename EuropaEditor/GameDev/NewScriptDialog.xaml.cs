using EnvDTE;
using EuropaEditor.GameProject.Backend;
using EuropaEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using EuropaProject = EuropaEditor.GameProject.Backend.Project;

namespace EuropaEditor.GameDev
{
    /// <summary>
    /// Interaction logic for NewScriptDialog.xaml
    /// </summary>
    public partial class NewScriptDialog : System.Windows.Window
    {
        //Templates for new script files.
        private static readonly string _cppCode =
        @"#include ""{0}.h""
        namespace {1}
        {{
            REGISTER_SCRIPT({0});
            void {0}::BeginPlay(float deltaTime)
            {{

            }}

            void {0}::Update(float deltaTime)
            {{

            }}
        }} // namespace {1}";

        private static readonly string _hCode =
        @"namespace {1} {{
	        class {0} : public Europa::Script::EntityScript {{
	            public:
		        constexpr explicit {0}(Europa::GameEntity::Entity entity) : Europa::Script::EntityScript(entity){{}}
                void BeginPlay() override
		        void Update(float deltaTime) override;
                private:

	        }};
        }}";

    public NewScriptDialog()
        {
            InitializeComponent();
        }

        bool Validate()
        {
            bool isValid = false;
            var name = scriptNameTextBox.Text.Trim();
            var path = scriptPathTextBox.Text.Trim();
            string errorMsg = string.Empty;
            if (string.IsNullOrEmpty(name))
            {
                errorMsg = "Type in a script name.";
            }
            else if(name.IndexOfAny(Path.GetInvalidPathChars()) != -1 || name.Any(x => char.IsWhiteSpace(x)))
            {
                errorMsg = "Invalid character(s) used in script name.";
            }
            if (string.IsNullOrEmpty(path))
            {
                errorMsg = "Select a valid script folder.";
            }
            else if (path.IndexOfAny(Path.GetInvalidFileNameChars()) != -1)
            {
                errorMsg = "Invalid character(s) used in script name.";
            }
            else if (!Path.GetFullPath(Path.Combine(EuropaProject.CurrentProject.Path, path)).Contains(Path.Combine(EuropaProject.CurrentProject.Path, @"GameCode\")))
            {
                errorMsg = "Script must be added to (a sub-folder of) GameCode.";
            }
            else if(File.Exists(Path.GetFullPath(Path.Combine(Path.Combine(EuropaProject.CurrentProject.Path, path), $"{name}.cpp"))) ||
                File.Exists(Path.GetFullPath(Path.Combine(Path.Combine(EuropaProject.CurrentProject.Path, path), $"{name}.h"))))
            {
                errorMsg = $"Script {name} already exists in this folder.";
            }
            else
            {
                isValid = true;
            }

            if(!isValid)
            {
                messageTextBlock.Foreground = FindResource("Editor.RedBrush") as Brush;
            }
            else
            {
                messageTextBlock.Foreground = FindResource("Editor.FontBrush") as Brush;
            }
            messageTextBlock.Text = errorMsg;
            return isValid;
        }

        private void OnScriptPath_TextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (!Validate())
                return;
            var name = scriptNameTextBox.Text.Trim();
            messageTextBlock.Text = $"{name}.h and {name}.cpp will be added to {EuropaProject.CurrentProject.Name}";
        }

        private void OnCreate_Button_Click(object sender, RoutedEventArgs e)
        {
            Validate();
        }

        private async void OnScriptName_TextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (!Validate())
                return;
            IsEnabled = false;
            try
            {
                await Task.Run(() => CreateScript(name, path, solution, projectName));
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.Log(MessageType.Error, $"Failed to create a script {scriptNameTextBox.Text}");
            }
        }

        private void CreateScript(string name, string path, string solution, string projectName)
        {
            if (!Directory.Exists(path))
                Directory.CreateDirectory(path);

            var cpp = Path.GetFullPath(Path.Combine(path, $"{name}.cpp"));
            var h = Path.GetFullPath(Path.Combine(path, $"{name}.h"));

            using (var sw = File.CreateText(cpp))
            {

            }

            using (var sw = File.CreateText(h))
            {

            }
        }
    }
}
