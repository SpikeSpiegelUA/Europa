using System;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows;

namespace YourEngineEditor
{
    public partial class GoapDialogueWindow : Window
    {
        // Link to the native engine backend
        private const string _engineDLL = @"C:\Projects\Europa\x64\DebugEditor\EngineDLL.dll";
        [DllImport(_engineDLL, CallingConvention = CallingConvention.Cdecl)]
        public static extern void CalculateAdvancedGOAP(
            int goalType,
            bool hasIdea,
            bool isTired,
            int location,
            StringBuilder outPlanBuffer,
            int bufferSize);

        public GoapDialogueWindow()
        {
            InitializeComponent();
        }

        private void OnComputePlanClicked(object sender, RoutedEventArgs e)
        {
            int selectedGoal = ComboBoxGoal.SelectedIndex;
            int selectedLocation = ComboBoxLocation.SelectedIndex;
            bool hasIdea = CheckBoxIdea.IsChecked ?? false;
            bool isTired = CheckBoxTired.IsChecked ?? false;

            StringBuilder buffer = new StringBuilder(1024);

            try
            {
                CalculateAdvancedGOAP(selectedGoal, hasIdea, isTired, selectedLocation, buffer, buffer.Capacity);
                TextBlockGOAPOutput.Text = buffer.ToString();
            }
            catch (Exception ex)
            {
                TextBlockGOAPOutput.Text = $"DLL Execution Error: {ex.Message}";
            }
        }
    }
}