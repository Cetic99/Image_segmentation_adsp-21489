import numpy as np
import cv2
from PIL import Image
import time
from matplotlib import pyplot as plt

def sobel_edge(im_name):
    start = time.time()
    # Read in the image and convert it to grayscale
    image = cv2.imread(im_name)
    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    #gray = cv2.GaussianBlur(gray,(5,5),0)
    
    # Apply the Sobel operator and compute the gradients in the x and y directions
    sobel_x = cv2.Sobel(gray, cv2.CV_64F, 1, 0, ksize=3)
    sobel_y = cv2.Sobel(gray, cv2.CV_64F, 0, 1, ksize=3)

    # Calculate the absolute values of the gradients and combine them to get the edges
    abs_sobel_x = cv2.convertScaleAbs(sobel_x)
    abs_sobel_y = cv2.convertScaleAbs(sobel_y)
    edges = cv2.addWeighted(abs_sobel_x, 0.5, abs_sobel_y, 0.5, 0)
    # Blur image using GaussianBlur
    #edges = cv2.GaussianBlur(edges,(3,3),0)

    cv2.imwrite("Edges.bmp",edges)
    end = time.time()
    print(end - start)
    return edges


# ENHANCED VERSION OF THIS IS IMPLEMENTED IN NATURAL C AND DSP ADSP-21489
def naive_connectedComponents(image):
    UINT32_MAX = 4294967295
    EDGE_VAL = 0
    height,width = np.shape(image)
    parent = [i for i in range(width*height)]        # everyone is its owm parent
    label_counter = np.uint32(1)
    min_neighbour = np.uint32(UINT32_MAX)
    matrix = np.ones((height,width),dtype=np.uint32)
    matrix = matrix * image                         #uint32 dtype
    for i in range(1,height-1):
        for j in range(1,width-1):
            min_neighbour = UINT32_MAX
            if image[i,j] == EDGE_VAL:
                continue
            if image[i-1,j-1] > EDGE_VAL and matrix[i-1,j-1] < min_neighbour:
                min_neighbour = matrix[i-1,j-1]
            if image[i-1,j] > EDGE_VAL and matrix[i-1,j] < min_neighbour:
                min_neighbour = matrix[i-1,j]
            if image[i-1,j+1] > EDGE_VAL and matrix[i-1,j+1] < min_neighbour:
                min_neighbour = matrix[i-1,j+1]
            if image[i,j-1] > EDGE_VAL and matrix[i,j-1] < min_neighbour:
                min_neighbour = matrix[i,j-1]
            
            if min_neighbour == UINT32_MAX:
                label_counter = label_counter + 1
                min_neighbour = label_counter
            matrix[i,j] = min_neighbour      # assigning this pixel its label value
            if image[i-1,j+1] > EDGE_VAL and matrix[i-1,j+1] > min_neighbour:
                parent[matrix[i-1,j+1]] = min_neighbour
            if image[i-1,j] > EDGE_VAL and matrix[i-1,j] > min_neighbour:
                parent[matrix[i-1,j]] = min_neighbour


    for i in range(label_counter):           # for each label find corresponding root parent
        j = i
        while(parent[j] != j):
            j = parent[j]
        parent[i] = j
    for i in range(1,height-1):
        for j in range(1,width-1):
            matrix[i,j] =parent[matrix[i,j]] 
    return matrix
            
def main():
    start = time.time()
    edges = sobel_edge('Image.bmp')    #uint8
    height, width = np.shape(edges)
    for i in range(height):
        for j in range(width):
            if edges[i,j] > 10:
                edges[i,j] = 0
            else:
                edges[i,j] = 1


    #labels_im = naive_connectedComponents(edges)        # (720,1280)
    _, labels_im = cv2.connectedComponents(edges)
    cv2.imshow('Labeled_gray',np.uint8(labels_im))
    def imshow_components(labels):
        # Map component labels to hue val
        label_hue = np.uint8(179*labels/np.max(labels))
        blank_ch = 255*np.ones_like(label_hue)
        labeled_img = cv2.merge([label_hue, blank_ch, blank_ch])

        # cvt to BGR for display
        labeled_img = cv2.cvtColor(labeled_img, cv2.COLOR_HSV2BGR)

        # set bg label to black
        labeled_img[label_hue == 0] = 0

        cv2.imshow('labeled.png', labeled_img)
        cv2.waitKey()
    imshow_components(labels_im)

    end = time.time()
    print(end - start)
            
if __name__ == '__main__':
    main()