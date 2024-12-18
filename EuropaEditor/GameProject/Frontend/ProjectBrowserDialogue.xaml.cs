﻿using EuropaEditor.GameProject.Backend;
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
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace EuropaEditor
{
    /// <summary>
    /// Interaction logic for ProjectBrowserDialogue.xaml
    /// </summary>
    public partial class ProjectBrowserDialogue : Window
    {
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

        private void OnToggleProjectButton_Click(object sender, RoutedEventArgs e)
        {
            if (sender == openProjectButton)
            {
                if (createProjectButton.IsChecked == true)
                {
                    createProjectButton.IsChecked = false;
                    projectActionsStackPanel.Margin = new Thickness(0.0);
                }
                openProjectButton.IsChecked = true;
            }
            else
            {
                if (openProjectButton.IsChecked == true)
                {
                    openProjectButton.IsChecked = false;
                    projectActionsStackPanel.Margin = new Thickness(-800.0, 0.0, 0.0, 0.0);
                }
                createProjectButton.IsChecked = true;
            }
        }
    }
}
