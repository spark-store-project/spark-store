# 有关 src 下的一些改进预览

> 1. 主体结构预览
> 2. 主体结构中的内容单独说明
> 并使用所用语言进行非侵入式独立描述，而不是在代码中填充说明与注释。
>
> 这是简单的预览说明规范

- widget/base 结构

    ```
    src/widgets/base/
    ├── basewidgetopacity.cpp
    └── basewidgetopacity.h

    0 directories, 2 files
    ```

- 来源于 widget/base 的说明 

    ```c++
    // BaseWidgetOpacity 是一个提供了淡出/淡入动画的基础类:
    //  1. closeEvent 窗口关闭时进行淡出动画
        // 此前在 MainWindow 中实现的淡出动画将由 BaseWidgetOpacity 来实现。

    // 此前 MainWindow 原有的 DBlurEffectWidget 父类将移交至 BaseWidgetOpacity 继承。
    
    // 注意: 
    // 如果 MainWindow 在未来重写 closeEvent 事件时将可能丢失 BaseWidgetOpacity 中的淡出效果
    ```