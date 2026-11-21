#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
内核源码收集脚本 - 简化版
只收集 .c, .h, .ld, .bat 格式的文件
输出到 output.txt
"""

import os
from pathlib import Path
from datetime import datetime

def collect_kernel_sources(source_dir="."):
    """
    收集指定目录下所有 .c, .h, .ld, .bat 文件到一个txt文件
    
    Args:
        source_dir: 源代码目录，默认为当前目录
    """
    
    # 指定要收集的文件扩展名
    target_extensions = {'.c', '.h'}
    
    # 忽略的目录
    ignore_dirs = {'.git', '.svn', '__pycache__', 'build', 'dist', 'obj', 'bin'}
    
    source_path = Path(source_dir)
    output_file = "output.txt"
    
    # 收集所有符合条件的文件
    collected_files = []
    
    print("正在扫描文件...")
    for root, dirs, files in os.walk(source_path):
        # 过滤掉忽略的目录
        dirs[:] = [d for d in dirs if d not in ignore_dirs]
        
        for file in files:
            file_path = Path(root) / file
            if file_path.suffix.lower() in target_extensions:
                relative_path = file_path.relative_to(source_path)
                collected_files.append((file_path, relative_path))
    
    # 按文件名排序
    collected_files.sort(key=lambda x: str(x[1]).lower())
    
    if not collected_files:
        print(f"未找到任何 .c, .h, .ld, .bat 文件在目录: {source_path.absolute()}")
        return
    
    print(f"找到 {len(collected_files)} 个文件")
    
    # 写入输出文件
    with open(output_file, 'w', encoding='utf-8') as out:
        # 写入文件头
        out.write("=" * 80 + "\n")
        out.write("内核源代码收集\n")
        out.write(f"生成时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
        out.write(f"源目录: {source_path.absolute()}\n")
        out.write(f"文件总数: {len(collected_files)}\n")
        out.write("=" * 80 + "\n\n")
        
        # 写入每个文件的内容
        for i, (file_path, relative_path) in enumerate(collected_files, 1):
            print(f"处理 ({i}/{len(collected_files)}): {relative_path}")
            
            # 写入文件分隔标记
            out.write("\n" + "*" * 80 + "\n")
            out.write(f"文件 {i}: {relative_path}\n")
            out.write("*" * 80 + "\n\n")
            
            # 读取并写入文件内容
            try:
                # 尝试不同编码
                content = None
                for encoding in ['utf-8', 'gbk', 'latin-1']:
                    try:
                        with open(file_path, 'r', encoding=encoding) as f:
                            content = f.read()
                        break
                    except UnicodeDecodeError:
                        continue
                
                if content is None:
                    out.write("[无法读取文件内容 - 编码不兼容]\n")
                else:
                    out.write(content)
                    if not content.endswith('\n'):
                        out.write('\n')
                    
                    # 统计行数
                    line_count = content.count('\n') + 1
                    out.write(f"\n[文件结束 - 共{line_count}行]\n")
                    
            except Exception as e:
                out.write(f"[读取文件错误: {str(e)}]\n")
            
            # 写入文件结束标记
            out.write("\n" + "-" * 80 + "\n\n")
        
        # 写入文件尾
        out.write("\n" + "=" * 80 + "\n")
        out.write("收集完成\n")
        out.write(f"总文件数: {len(collected_files)}\n")
        out.write(f"完成时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
        out.write("=" * 80 + "\n")
    
    print(f"\n✅ 成功生成 {output_file}")
    print(f"   包含 {len(collected_files)} 个文件")
    print(f"   文件大小: {Path(output_file).stat().st_size} 字节")
    
    # 显示收集到的文件列表
    print("\n收集的文件:")
    for i, (_, relative_path) in enumerate(collected_files, 1):
        print(f"  {i:3d}. {relative_path}")

if __name__ == "__main__":
    import sys
    
    # 可以接受命令行参数指定源目录
    if len(sys.argv) > 1:
        source_directory = sys.argv[1]
    else:
        source_directory = "."
    
    print("内核源码收集工具")
    print(f"目标格式: .c, .h, .ld, .bat")
    print(f"输出文件: output.txt")
    print("=" * 50)
    
    collect_kernel_sources(source_directory)
