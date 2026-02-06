using EuropaEditor.Content;
using EuropaEditor.ContentToolsAPIStructs;
using EuropaEditor.DLLWrapper;
using EuropaEditor.Editors;
using EuropaEditor.Utilities.Controls;
using System;
using System.Collections.Generic;
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
using System.Windows.Shapes;

namespace EuropaEditor.Windows
{
    /// <summary>
    /// Interaction logic for PrimitiveMeshDialogue.xaml
    /// </summary>
    public partial class PrimitiveMeshDialogue : Window
    {

        private static readonly List<ImageBrush> _textures = new List<ImageBrush>();
        static PrimitiveMeshDialogue()
        {
            LoadTextures();
        }

        public PrimitiveMeshDialogue()
        {
            InitializeComponent();
            Loaded += (s, e) => UpdatePrimitive();
        }

        private static void LoadTextures()
        {
            var uris = new List<Uri>
            {
                new Uri("pack://application:,,,/Resources/PrimitiveMeshView/PlaneTexture.png"),
                new Uri("pack://application:,,,/Resources/PrimitiveMeshView/PlaneTexture.png"),
                new Uri("pack://application:,,,/Resources/PrimitiveMeshView/PlaneTexture.png"),
                new Uri("pack://application:,,,/Resources/PrimitiveMeshView/PlaneTexture.png")
            };

            _textures.Clear();
            foreach(var uri in uris)
            {
                var resource = Application.GetResourceStream(uri);
                using var reader = new BinaryReader(resource.Stream);
                var data = reader.ReadBytes((int)resource.Stream.Length);
                var imageSource = (BitmapSource)new ImageSourceConverter().ConvertFrom(data);
                imageSource.Freeze();
                var brush = new ImageBrush(imageSource);
                brush.Transform = new ScaleTransform(1, -1, 0.5, 0.5);
                brush.ViewportUnits = BrushMappingMode.Absolute;
                brush.Freeze();
                _textures.Add(brush);
            }
        }

        private void OnPrimitiveType_ComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e) => UpdatePrimitive();

        private void OnSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e) => UpdatePrimitive();

        private float Value(ScalarBox scalarBox, float min) {
            float.TryParse(scalarBox.Value, out var result);
            return Math.Max(result, min);
        }

        private void OnScalarBox_ValueChanged(object sender, RoutedEventArgs e) => UpdatePrimitive();

        private void UpdatePrimitive()
        {
            if (!IsInitialized) return;

            var primitiveType = (PrimitiveMeshType)PrimTypeComboBox.SelectedItem;
            var info = new PrimitiveInitInfo { Type = primitiveType };

            switch (primitiveType)
            {
                case PrimitiveMeshType.Plane:
                    {
                        info.SegmentX = (int)XSliderPlane.Value;
                        info.SegmentZ = (int)ZSliderPlane.Value;
                        info.Size.X = Value(WidthScalarBoxPlane, 0.001f);
                        info.Size.Z = Value(LengthScalarBoxPlane, 0.001f);
                    }
                    break;
                case PrimitiveMeshType.Cube:
                    return;                
                case PrimitiveMeshType.UVSphere:
                    {
                        info.SegmentX = (int)XSliderUVSphere.Value;
                        info.SegmentY = (int)YSliderUVSphere.Value;
                        info.Size.X = Value(XScalarBoxUVSphere, 0.001f);
                        info.Size.Y = Value(YScalarBoxUVSphere, 0.001f);
                        info.Size.Z = Value(ZScalarBoxUVSphere, 0.001f);
                    }
                    break;
                case PrimitiveMeshType.IcoSphere:
                    return;
                case PrimitiveMeshType.Cylinder:
                    return;
                case PrimitiveMeshType.Capsule:
                    return;
                default:
                    break;
            }
            var geometry = new Content.Geometry();
            ContentToolsAPI.CreatePrimitiveMesh(geometry, info);
            (DataContext as GeometryEditor).SetAsset(geometry);
            OnTexture_CheckBox_Click(textureCheckBox, null);
        }

        private void OnTexture_CheckBox_Click(object sender, RoutedEventArgs e)
        {
            Brush brush = Brushes.White;
            if((sender as CheckBox).IsChecked == true)
            {
                brush = _textures[(int)PrimTypeComboBox.SelectedItem];
            }

            var vm = DataContext as GeometryEditor;
            foreach(var mesh in vm.MeshRenderer.Meshes)
            {
                mesh.Diffuse = brush;
            }
        }
    }
}
