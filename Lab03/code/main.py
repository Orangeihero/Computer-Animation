import numpy as np
import math
import cv2
import matplotlib
from matplotlib import pyplot
from matplotlib import patches
import random
import os

#----------------------------------------------------------------------------------------
#-------------------------------------计算点应有的速度-------------------------------------
#--------------------------------------计算首选的速度--------------------------------------
#--------------------------------------计算新的速度--------------------------------------
#----------------------------------------------------------------------------------------

# 给定两个点，计算这两个点之间的距离
def getDistance(position1, position2):
    distance = np.sqrt((position1[0] - position2[0]) ** 2 + (position1[1] - position2[1]) ** 2)+0.001
    return distance


# 计算首选速度
def getPrefVelocity(robot):
    pref_v = []  # 首选速度
    for i in range(len(robot['current_position'])):
        distance = getDistance(robot['current_position'][i], robot['goal_position'][i])  # 现在所在位置与目标位置之间的距离
        difference_x = robot['goal_position'][i][0] - robot['current_position'][i][0]  # 现在所在位置与目标位置之间横坐标的差
        difference_y = robot['goal_position'][i][1] - robot['current_position'][i][1]  # 现在所在位置与目标位置之间纵坐标的差
        if (distance <= 0.1):  # 已经接近终点
            pref_v.append([0.0, 0.0])
        else:  # 尚未接近终点
            difference = [difference_x, difference_y]  # 现在所在位置与目标位置之间横坐标、纵坐标的差的列表表示
            pref_v_x = difference[0] * robot['max_velocity'][i] / distance
            pref_v_y = difference[1] * robot['max_velocity'][i] / distance
            pref_v.append([pref_v_x, pref_v_y])
    return pref_v


# 计算新速度
def getNewVelocity(robot, obstacle, pref_velocity):
    new_v = robot['current_velocity']  # 新的速度列表，赋予初值即为当前速度
    # 遍历所有的点，找到所有点的速度，当前点称为点A
    for i in range(len(robot['current_position'])):
        radius_A = robot['radius'][i] + 0.1  # 点A的半径
        v_A = robot['current_velocity'][i]  # 点A的当前速度
        position_A = robot['current_position'][i]  # 点A的当前位置
        rel_BA = []  # 点A与B的关联信息

        # 要考虑到除本身之外的所有点
        for j in range(len(robot['current_position'])):
            if j != i:
                radius_B = robot['radius'][j] + 0.1  # 点B的半径
                v_B = robot['current_velocity'][j]  # 点B的当前速度
                position_B = robot['current_position'][j]  # 点B的当前位置
                # 点A与点B之间的关系
                RVO_BA = [position_A[0] + 0.5 * (v_A[0] + v_B[0]), position_A[1] + 0.5 * (v_A[1] + v_B[1])]  # RVO算法
                left_angle, right_angle, radius_AB,distance_BA = getRela(position_A, position_B, radius_A, radius_B, v_A, v_B)
                rel_BA.append([RVO_BA,left_angle, right_angle, radius_AB,distance_BA])
        # 需要考虑所有的障碍物
        if obstacle:
            for k in range(len(obstacle['position'])):
                v_B = [0.0, 0.0]
                position_B = obstacle['position'][k]
                if obstacle['type'] == 'circle':  # 如果障碍物是圆形的
                    radius_B = obstacle['radius'][k][0] + 0.1
                elif obstacle['type'] == 'rectangle':  # 如果障碍物为长方形或正方形，需要计算出到对角线的距离（近似）
                    radius_B = max(obstacle['radius'][k][0], obstacle['radius'][k][1]) * 1.5
                RVO_BA = [position_A[0] + v_B[0], position_A[1] + v_B[1]]  # RVO算法
                left_angle, right_angle, radius_AB, distance_BA = getRela(position_A, position_B, radius_A, radius_B,
                                                                          v_A, v_B)
                rel_BA.append([RVO_BA, left_angle, right_angle, radius_AB, distance_BA])

        new_v[i] = selectVelocity(position_A, pref_velocity[i], rel_BA)

    return new_v

#得到个体A和个体B之间的一些关系
def getRela(position_A, position_B, radius_A, radius_B, v_A, v_B):

    distance_BA = getDistance(position_A, position_B)  # AB之间的距离
    angle_BA = math.atan2(position_B[1] - position_A[1], position_B[0] - position_A[0])  # AB圆心的连线与水平线之间的角度大小

    # 如果AB之间的距离小于其半径之和，则用半径之和代替距离
    if distance_BA < radius_A + radius_B:
        distance_BA = radius_A + radius_B
    half_angle_between_BA = math.asin((radius_B + radius_A) / distance_BA)  # 切线与两线连线的夹角
    # 三角锥的左边界
    left_angle = angle_BA + half_angle_between_BA
    left_angle = math.atan2(math.sin(left_angle), math.cos(left_angle))  # 使角度一定处于atan2所允许的范围内（-pi，pi）
    # 三角锥的右边界
    right_angle = angle_BA - half_angle_between_BA
    right_angle = math.atan2(math.sin(right_angle), math.cos(right_angle))  # 使角度一定处于atan2所允许的范围内（-pi，pi）
    return left_angle, right_angle, radius_A + radius_B, distance_BA


#选择新速度
def selectVelocity(position_A, pref_vA, rel_BA):
    suit_v = []  # 符合要求的速度列表
    pref_vA_cons = getDistance(pref_vA, [0.0, 0.0])  # A首选速度的常数值

    # 首先测试所有的中间插值
    for angle in np.arange(0, 2 * math.pi, 0.1):
        for v in np.arange(0.02, pref_vA_cons + 0.02, pref_vA_cons / 5.0):
            new_v = [v * math.cos(angle), v * math.sin(angle)]
            isSuit = judgeSuit(new_v, position_A, rel_BA)
            if isSuit:
                suit_v.append(new_v)
    # 测试首选速度
    new_v = pref_vA
    isSuit = judgeSuit(new_v, position_A, rel_BA)
    if isSuit:
        suit_v.append(new_v)

    # 处理符合要求的速度和不符合要求的速度
    if suit_v:
        new_v = min(suit_v, key=lambda v: getDistance(v, pref_vA))
    else:
        new_v = [0.0, 0.0]

    return new_v

# 判断速度是否符合条件
def judgeSuit(new_v, position_A, rel_BA):
    isSuit = True
    for i in range(len(rel_BA)):
        position_AB = rel_BA[i][0]
        left_angle = rel_BA[i][1]
        right_angle = rel_BA[i][2]
        difference = [new_v[0] + position_A[0] - position_AB[0], new_v[1] + position_A[1] - position_AB[1]]
        diff_angle = math.atan2(difference[1], difference[0])
        # 如果相对速度处于碰撞区域内，则不符合条件
        if isBetween(left_angle, right_angle, diff_angle):
            isSuit = False
            break
    return isSuit

#判断相对速度是否处于碰撞区域内
def isBetween(left_angle, right_angle, diff_angle):
    if abs(right_angle - left_angle) <= math.pi:
        if right_angle <= diff_angle <= left_angle:
            return True
        else:
            return False
    else:
        if (left_angle < 0) and (right_angle > 0):
            # print('impossible')
            left_angle += 2 * math.pi
            if diff_angle < 0:
                diff_angle += 2 * math.pi
            if right_angle <= diff_angle <= left_angle:
                return True
            else:
                return False
        if (left_angle > 0) and (right_angle < 0):
            # print('is it possible?')
            right_angle += 2 * math.pi
            if diff_angle < 0:
                diff_angle += 2 * math.pi
            if left_angle <= diff_angle <= right_angle:
                return True
            else:
                return False

#----------------------------------------------------------------------------------------
#------------------------------每隔0.4s绘制一帧图片（每秒25帧）-----------------------------
#-----------------------------------------得到视频----------------------------------------
#----------------------------------------------------------------------------------------

def drawProcess(size,robot, obstacle, color, new_velocity, time):
    figure = pyplot.figure() #建立窗口
    ax = figure.add_subplot(1, 1, 1) #添加子窗口
    pyplot.axis('off') #隐藏坐标轴
    if obstacle: #如果存在障碍物
        if obstacle['type'] == 'rectangle':
            for i in range(len(obstacle['position'])):
                ob = matplotlib.patches.Rectangle(
                    (obstacle['position'][i][0] - obstacle['radius'][i][0], obstacle['position'][i][1] - obstacle['radius'][i][1]),
                    2 * obstacle['radius'][i][0], 2 * obstacle['radius'][i][1],
                    facecolor='red',
                    fill=True,
                    alpha=1)
                ax.add_patch(ob)
        if obstacle['type'] == 'circle':
            for i in range(len(obstacle['position'])):
                ob = matplotlib.patches.Circle(
                    (obstacle['position'][i][0], obstacle['position'][i][1]),
                    radius=obstacle['radius'][i][0],
                    facecolor='red',
                    fill=True,
                    alpha=1)
                ax.add_patch(ob)
    for i in range(len(robot['current_position'])):
        r = matplotlib.patches.Circle(
            (robot['current_position'][i][0], robot['current_position'][i][1]),
            radius=robot['radius'][0],
            facecolor=color[i],
            edgecolor='black',
            linewidth=1.0,
            ls='solid',
            alpha=1,
            zorder=2)
        ax.add_patch(r)

    ax.set_aspect('equal')
    ax.set_xlim(size[0], size[1])
    ax.set_ylim(size[0], size[1])

    str_time = str(time)
    time_final='{0:0>3}'.format(str_time)

    pyplot.savefig('./image/'+time_final+'.png')
    pyplot.cla()
    pyplot.close(figure)
    return figure

#获取随机颜色
def getColor(N):
    color_arr = ['1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F']
    color = []
    for i in range(N):
        c = '#'
        for i in range(6):
            c += color_arr[random.randint(0, 14)]
        color.append(c)
    return color

def getVideo(path):
    #定义视频参数
    fps = 25
    video_name = './video/video.avi'
    out = cv2.VideoWriter(video_name, cv2.VideoWriter_fourcc(*'XVID'), fps, (640, 480))
    image_list = os.listdir(path)
    for i in range(len(image_list)):
        image = cv2.imread(path + image_list[i])
        out.write(image)
    cv2.destroyAllWindows()

#----------------------------------------------------------------------------------------
#--------------------------------根据用户的输入选择不同的场景-------------------------------
#----------------------------------------------------------------------------------------

def getModel(case):
    robot = dict()
    obstacle = dict()
    if case == '1':
        robot['current_position'] = [[-5.0, -5.0+1.0*i] for i in range(10)] + [[5.0, -5.0+1.0*i] for i in range(10)]
        robot['goal_position'] = [[5.0, 5.0-1.0*i] for i in range(10)] + [[-5.0, 5.0-1.0*i] for i in range(10)]
        robot['radius'] = [0.2 for i in range(len(robot['current_position']))]
        robot['current_velocity'] = [[0.0, 0.0] for i in range(len(robot['current_position']))]
        robot['max_velocity'] = [5.0 for i in range(len(robot['current_position']))]
        x1 = -6
        x2 = 6
    if case == '2':
        robot['current_position'] = []
        robot['goal_position'] = []
        for theta in np.arange(0,360,10):
            angle = math.radians(theta)
            # print(angle)
            robot['current_position'].append([8.0*math.cos(angle),8.0*math.sin(angle)])
            robot['goal_position'].append([-8.0*math.cos(angle),-8.0*math.sin(angle)])
        robot['radius'] = [0.2 for i in range(len(robot['current_position']))]
        robot['current_velocity'] = [[0.0, 0.0] for i in range(len(robot['current_position']))]
        robot['max_velocity'] = [5.0 for i in range(len(robot['current_position']))]

        x1 = -10
        x2 = 10
    if case == '3' or case == '4':
        robot['current_position'] = [[-5.0, -5.0 + 1.0 * i] for i in range(10)] + [[5.0, -5.0 + 1.0 * i] for i in
                                                                                   range(10)]
        robot['goal_position'] = [[5.0, 5.0 - 1.0 * i] for i in range(10)] + [[-5.0, 5.0 - 1.0 * i] for i in range(10)]
        robot['radius'] = [0.2 for i in range(len(robot['current_position']))]
        robot['current_velocity'] = [[0.0, 0.0] for i in range(len(robot['current_position']))]
        robot['max_velocity'] = [5.0 for i in range(len(robot['current_position']))]

        if case == '4':
            obstacle['type'] = 'circle'
        else:
            obstacle['type'] = 'rectangle'
        obstacle['radius'] = [[0.5, 0.5], [0.5, 0.5],[0.5, 0.5],[0.5, 0.5]]
        obstacle['position'] = [[-1.5, 1.5], [1.5, 1.5], [1.5, -1.5], [-1.5, -1.5]]
        x1 = -7
        x2 = 7
    return robot, obstacle,(x1,x2)


# 给定所有点当前的坐标和最终的坐标，判断是否所有点全部到达
def isArrived(current_position, goal_position):
    for i in range(len(current_position)):
        distance = getDistance(current_position[i], goal_position[i])
        if distance > 0.1:
            return False
    return True

def makeDir(path):
    isExists = os.path.exists(path)
    if not isExists:
        os.makedirs(path)

def main():
    case = input(
        'id\t\tmodel\n1\t2 lines and no obstacle\n2\t1 circle and no obstacle\n3\t20 points and 4 obstacles(rectangle)\n4\t20 points and 4 obstacles(circle)\nPlease input the id of the model:\n')
    robot, obstacle, size = getModel(case)
    makeDir('image')
    makeDir('video')
    t = 0
    color = getColor(len(robot['current_position']))
    while not isArrived(robot['current_position'], robot['goal_position']):
        pref_velocity = getPrefVelocity(robot)
        new_velocity = getNewVelocity(robot, obstacle, pref_velocity)
        drawProcess(size, robot, obstacle, color, new_velocity, t)
        t += 1
        for i in range(len(robot['current_position'])):
            robot['current_position'][i][0] += new_velocity[i][0] * 0.04
            robot['current_position'][i][1] += new_velocity[i][1] * 0.04
            drawProcess(size, robot, obstacle, color, new_velocity, t)
    getVideo('./image/')

if __name__ == "__main__":
    main()
