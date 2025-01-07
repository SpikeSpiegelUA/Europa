using EuropaEditor.EngineAPIStructs;
using EuropaEditor.Сomponents;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

//Here are declared all the structs that we need to communicated with the EngineDLL.
namespace EuropaEditor.EngineAPIStructs
{
    [StructLayout(LayoutKind.Sequential)]
    class APITransformComponent
    {
        public Vector3 Position;
        public Vector3 Rotation;
        public Vector3 Scale = new Vector3(1, 1, 1);
    }

    [StructLayout(LayoutKind.Sequential)]
    class GameEntityDescriptor
    {
        public APITransformComponent TransformComponent = new APITransformComponent();
    }
}

//Here are imported and wrapped all the DLL functions.
namespace EuropaEditor.DLLWrapper
{
    static class EngineAPI
    {
        private const string _dllName = "EngineDLL.dll";

        [DllImport(_dllName)]
        private static extern int CreateGameEntity(EngineAPIStructs.GameEntityDescriptor gameEntityDescriptor);

        public static int CreateGameEntity(GameEntity gameEntity)
        {
            GameEntityDescriptor gameEntityDescriptor = new GameEntityDescriptor();

            //Fill a transform component.
            { 
                var entityTransform = gameEntity.GetComponent<TransformComponent>();
                gameEntityDescriptor.TransformComponent.Position = entityTransform.Position;
                gameEntityDescriptor.TransformComponent.Rotation = entityTransform.Rotation;
                gameEntityDescriptor.TransformComponent.Scale = entityTransform.Scale;
            }
            
            return CreateGameEntity(gameEntityDescriptor);
        }

        [DllImport(_dllName)]
        private static extern void RemoveGameEntity(int id);

        public static void RemoveGameEntity(GameEntity gameEntity)
        {
            RemoveGameEntity(gameEntity.EntityID);
        }
    }
}
