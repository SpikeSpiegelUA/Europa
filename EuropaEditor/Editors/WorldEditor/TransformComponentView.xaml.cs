using EuropaEditor.Editors.WorldEditor;
using EuropaEditor.GameProject.Backend;
using EuropaEditor.Utilities;
using EuropaEditor.Сomponents;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Numerics;
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

namespace EuropaEditor.Editors
{
    /// <summary>
    /// Interaction logic for TransformComponentView.xaml
    /// </summary>
    public partial class TransformComponentView : UserControl
    {
        private Action _undoAction = null;
        private bool _propertyChanged = false;
        public TransformComponentView()
        {
            InitializeComponent();
            Loaded += OnTransformView_Loaded;
        }

        private void OnTransformView_Loaded(object sender, RoutedEventArgs e)
        {
            Loaded -= OnTransformView_Loaded;
            (DataContext as MSTransformComponent).PropertyChanged += (s, ee) => _propertyChanged = true;
        }

        private Action GetAction(Func<TransformComponent, (TransformComponent transform, Vector3)> selector, 
            Action<(TransformComponent transform, Vector3)> forEachAction)
        {
            if (!(DataContext is MSTransformComponent msTransformComponent))
            {
                _undoAction = null;
                return null;
            }
            var selection = msTransformComponent.SelectedComponents.Select(transform => (transform, transform.Position)).ToList();
            return new Action(() =>
            {
                selection.ForEach(item => item.transform.Position = item.Position);
                (GameEntityView.Instance.DataContext as MSEntity)?.GetMSComponent<MSTransformComponent>().Refresh();
            });
        }

        private Action GetPositionAction() => GetAction(x => (x, x.Position), (x) => x.transform.Position = x.Item2);
        private Action GetRotationAction() => GetAction(x => (x, x.Rotation), (x) => x.transform.Rotation = x.Item2);
        private Action GetScaleAction() => GetAction(x => (x, x.Scale), (x) => x.transform.Scale = x.Item2);

        private void RecordAction(Action redoAction, string name)
        {
            if (_propertyChanged)
            {
                Debug.Assert(_undoAction != null);
                _propertyChanged = false;
                Project.UndoRedoManager.Add(new UndoRedoAction(_undoAction, redoAction, name));
            }
        }

        private void OnPosition_VectorBox_PreviewMouseLBD(object sender, MouseButtonEventArgs e)
        {
            _propertyChanged = false;
            _undoAction = GetPositionAction();
        }

        private void OnPosition_VectorBox_PreviewMouseLBU(object sender, MouseButtonEventArgs e)
        {
            RecordAction(GetPositionAction(), "Position changed");
        }

        private void OnRotation_VectorBox_PreviewMouseLBD(object sender, MouseButtonEventArgs e)
        {
            _propertyChanged = false;
            _undoAction = GetRotationAction();
        }

        private void OnRotation_VectorBox_PreviewMouseLBU(object sender, MouseButtonEventArgs e)
        {
            RecordAction(GetRotationAction(), "Rotation changed");
        }

        private void OnScale_VectorBox_PreviewMouseLBD(object sender, MouseButtonEventArgs e)
        {
            _propertyChanged = false;
            _undoAction = GetScaleAction();
        }

        private void OnScale_VectorBox_PreviewMouseLBU(object sender, MouseButtonEventArgs e)
        {
            RecordAction(GetScaleAction(), "Scale changed");
        }

        private void OnPosition_VectorBox_LostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (_propertyChanged && _undoAction != null)
            {
                OnPosition_VectorBox_PreviewMouseLBU(sender, null);
            }
        }

        private void OnRotation_VectorBox_LostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (_propertyChanged && _undoAction != null)
            {
                OnRotation_VectorBox_PreviewMouseLBU(sender, null);
            }
        }

        private void OnScale_VectorBox_LostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (_propertyChanged && _undoAction != null)
            {
                OnScale_VectorBox_PreviewMouseLBU(sender, null);
            }
        }
    }
}
