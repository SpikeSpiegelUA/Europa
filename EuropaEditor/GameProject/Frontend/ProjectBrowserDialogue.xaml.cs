using EuropaEditor.GameProject.Backend;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace EuropaEditor
{
    /// <summary>
    /// Interaction logic for ProjectBrowserDialogue.xaml
    /// </summary>
    public partial class ProjectBrowserDialogue : Window
    {
        private readonly CubicEase _cubicEasing = new CubicEase() { EasingMode = EasingMode.EaseInOut };

        public ProjectBrowserDialogue()
        {
            InitializeComponent();
            Loaded += OnProjectBrowserDialogue_Loaded;
        }

        private void OnProjectBrowserDialogue_Loaded(object sender, RoutedEventArgs e)
        {
            Loaded -= OnProjectBrowserDialogue_Loaded;
            if (!OpenProject.Projects.Any())
            {
                openProjectButton.IsEnabled = false;
                openProjectView.Visibility = Visibility.Hidden;
                OnToggleProjectButton_Click(createProjectButton, new RoutedEventArgs());
            }
        }

        private void AnimateToCreateProject()
        {
            var highlightAnimation = new DoubleAnimation(230, 400, new Duration(TimeSpan.FromSeconds(0.2)));
            //highlightAnimation.EasingFunction = _cubicEasing;
            highlightAnimation.Completed += (s, e) =>
            {
                var thicknessAnimation = new ThicknessAnimation(new Thickness(0), new Thickness(-1600.0, 0.0, 0.0, 0.0), 
                    new Duration(TimeSpan.FromSeconds(0.5)));
                thicknessAnimation.EasingFunction = _cubicEasing;
                projectActionsStackPanel.BeginAnimation(MarginProperty, thicknessAnimation);
            };
            highLightRectangle.BeginAnimation(Canvas.LeftProperty, highlightAnimation);
        }

        private void AnimateToOpenProject()
        {
            var highlightAnimation = new DoubleAnimation(400, 230, new Duration(TimeSpan.FromSeconds(0.2)));
            //highlightAnimation.EasingFunction = _cubicEasing;
            highlightAnimation.Completed += (s, e) =>
            {
                var thicknessAnimation = new ThicknessAnimation(new Thickness(-1600.0, 0.0, 0.0, 0.0),
                    new Thickness(0), new Duration(TimeSpan.FromSeconds(0.5)));
                thicknessAnimation.EasingFunction = _cubicEasing;
                projectActionsStackPanel.BeginAnimation(MarginProperty, thicknessAnimation);
            };
            highLightRectangle.BeginAnimation(Canvas.LeftProperty, highlightAnimation);
        }

        private void OnToggleProjectButton_Click(object sender, RoutedEventArgs e)
        {
            if (sender == openProjectButton)
            {
                if (createProjectButton.IsChecked == true)
                {
                    createProjectButton.IsChecked = false;
                    AnimateToOpenProject();
                    openProjectView.IsEnabled = true;
                    newProjectView.IsEnabled = false;
                }
                openProjectButton.IsChecked = true;
            }
            else
            {
                if (openProjectButton.IsChecked == true)
                {
                    openProjectButton.IsChecked = false;
                    AnimateToCreateProject();
                    openProjectView.IsEnabled = false;
                    newProjectView.IsEnabled = true;
                }
                createProjectButton.IsChecked = true;
            }
        }
    }
}
