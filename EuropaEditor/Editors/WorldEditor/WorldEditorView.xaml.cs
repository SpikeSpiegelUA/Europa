using EuropaEditor.GameProject.Backend;
using System;
using System.Collections.Generic;
using System.Collections.Specialized;
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
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace EuropaEditor.Editors.WorldEditor
{
    /// <summary>
    /// Interaction logic for WorldEditorView.xaml
    /// </summary>
    public partial class WorldEditorView : UserControl
    {
        public WorldEditorView()
        {
            InitializeComponent();
            Loaded += OnWorldEditor_Loaded;
        }

        private void OnWorldEditor_Loaded(object sender, RoutedEventArgs e)
        {
            Loaded -= OnWorldEditor_Loaded;
            Focus();
            ((INotifyCollectionChanged)Project.UndoRedoManager.UndoList).CollectionChanged += (s, d) => Focus();
        }
    }
}
