using EuropaEditor.Utilities;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace EuropaEditor.GameDev
{
    static class VisualStudio
    {
        private static EnvDTE80.DTE2 _vsInstance = null;
        private static readonly string _progID = "VisualStudio.DTE.17.0";
        public static void OpenVisualStudio(string solutionPath)
        {
            try
            {
                if (_vsInstance == null)
                {
                    Type visualStudioType = Type.GetTypeFromProgID(_progID, true);
                    _vsInstance = Activator.CreateInstance(visualStudioType) as EnvDTE80.DTE2;
                }
            }
            catch(Exception ex)
            {
                Debug.WriteLine(ex.Message);
                Logger.Log(MessageType.Error, "Failed to open the Visual Studio");
            }
        }
    }
}
