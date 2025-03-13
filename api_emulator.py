from flask import Flask, request, jsonify
from datetime import datetime
import logging

# 配置日志
logging.basicConfig(level=logging.INFO, 
                    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

app = Flask(__name__)

@app.route('/TOOLS-M-AlarmMessage/send', methods=['POST'])
def send_alarm_message():
    """
    模拟短信发送接口
    
    请求格式:
    {
        "deviceId": "",  // 设备编码
        "alarmTime": "", // 预警时间
        "alarmMsg": ""   // 预警短信内容
    }
    
    返回格式:
    {
        "code": "200",  // 200成功，500失败
        "message": "success"
    }
    """
    try:
        # 获取请求数据
        data = request.json
        
        # 检查必要字段
        if not all(key in data for key in ['deviceId', 'alarmTime', 'alarmMsg']):
            logger.error("Missing required fields in request")
            return jsonify({
                "code": "500",
                "message": "Missing required fields"
            }), 400
        
        # 记录接收到的请求
        logger.info(f"Received alarm message request:")
        logger.info(f"Device ID: {data['deviceId']}")
        logger.info(f"Alarm Time: {data['alarmTime']}")
        logger.info(f"Alarm Message: {data['alarmMsg']}")
        
        # 模拟处理时间
        # time.sleep(0.5)  # 取消注释以模拟处理延迟
        
        # 返回成功响应
        return jsonify({
            "code": "200",
            "message": "success"
        })
        
    except Exception as e:
        logger.error(f"Error processing request: {str(e)}")
        return jsonify({
            "code": "500",
            "message": f"Error: {str(e)}"
        }), 500

# 添加一个简单的健康检查端点
@app.route('/health', methods=['GET'])
def health_check():
    return jsonify({
        "status": "up",
        "timestamp": datetime.now().isoformat()
    })

if __name__ == '__main__':
    logger.info("Starting alarm message mock server...")
    app.run(host='0.0.0.0', port=5000, debug=True)
