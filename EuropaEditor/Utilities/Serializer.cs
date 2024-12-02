using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Threading.Tasks;

namespace EuropaEditor.Utilities
{
   public static class Serializer
    {
        public static void ToFile<T>(T instance, string path)
        {
            try
            {
                var fileStream = new FileStream(path, FileMode.Create);
                var serializer = new DataContractSerializer(typeof(T));
                serializer.WriteObject(fileStream, instance);
            }
            catch(Exception ex)
            {
                Debug.WriteLine(ex.Message);
                //TODO: log error.
            }
        }
    }
}
