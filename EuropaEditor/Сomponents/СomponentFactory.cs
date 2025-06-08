using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace EuropaEditor.Сomponents
{
    enum ComponentType
    {
        Transform,
        Script
    }
    internal static class ComponentFactory
    {
        private static readonly Func<GameEntity, object, Component>[] _function = new Func<GameEntity, object, Component>[]
        {
            (entity, data) => new TransformComponent(entity),
            (entity, data) => new ScriptComponent(entity){Name = (string)data},
        };
        public static Func<GameEntity, object, Component> GetCreationFunction(ComponentType type)
        {
            Debug.Assert((int)type < _function.Length);
            return _function[(int)type];
        }
        public static ComponentType ToEnumType(this Component component)
        {
            return component switch
            {
                TransformComponent _ => ComponentType.Transform,
                ScriptComponent _ => ComponentType.Script,
                _ => throw new ArgumentException("Unknown component type"),
            };
        }
    }
}
