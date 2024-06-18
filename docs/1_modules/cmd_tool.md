# 命令行工具

## Parameters Tool
> 用于快速处理参数配置，包括查询、修改、删除等操作。
> 
> 工具脚本：`tbk/tools/tool_param.py`

- 使用示例
    - list all parameters
    ```
    tool_param.py list
    ```
    - get params in a particular group
    ```
    tool_param.py list {param_prefix}
    ```
    - get a particular param
    ```
    tool_param.py get {param_name}
    ```
    - set a particular param
    ```
    tool_param.py set {param_name} {value}
    ```
    - save all params to a file
    ```
    tool_param.py save {file_name}
    ```
    - load all params from a file
    ```
    tool_param.py load {file_name}
    ```
