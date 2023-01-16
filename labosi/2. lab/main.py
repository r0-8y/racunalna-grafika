import math
import random
from math import acos
from typing import List, Any

from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *
import pygame as pg

WIDTH = 512
HEIGHT = 512


class Vertex:
    def __init__(self, x: float, y: float, z: float):
        self.x = x
        self.y = y
        self.z = z

    def copy(self):
        return Vertex(self.x, self.y, self.z)


class Source:
    def __init__(self, vertex, c, size):
        self.pos = vertex
        self.c_r = c[0]
        self.c_g = c[1]
        self.c_b = c[2]
        self.size = size


class Particle:
    def __init__(self, direction, v, source: Source):
        self.source = source
        self.pos = source.pos.copy()
        self.r = source.c_r
        self.g = source.c_g
        self.b = source.c_b
        self.v = v
        self.t = random.randint(70, 90)
        self.sx = direction.x
        self.sy = direction.y
        self.sz = direction.z
        self.vectorx = 0
        self.vectory = 0
        self.vectorz = 0
        self.angle = 0
        self.size = source.size

    def draw_particle(self):
        glColor3f(self.r, self.g, self.b)
        glTranslatef(self.pos.x, self.pos.y, self.pos.z)
        glRotatef(self.angle, self.vectorx, self.vectory, self.vectorz)

        glBegin(GL_QUADS)
        glTexCoord2d(0.0, 0.0)
        glVertex3f(-self.size, -self.size, 0.0)
        glTexCoord2d(1.0, 0.0)
        glVertex3f(self.size, -self.size, 0.0)
        glTexCoord2d(1.0, 1.0)
        glVertex3f(self.size, self.size, 0.0)
        glTexCoord2d(0.0, 1.0)
        glVertex3f(-self.size, self.size, 0.0)
        glEnd()

        glRotatef(-self.angle, self.vectorx, self.vectory, self.vectorz)
        glTranslatef(-self.pos.x, -self.pos.y, -self.pos.z)

    def change_particle_position(self):
        self.pos.x += self.v * self.sx
        self.pos.y += self.v * self.sy
        self.pos.z += self.v * self.sz

    def change_particle_color_and_size(self):
        if self.b < 1:
            self.b += 0.01
        self.size += 0.03

    def set_vector(self, vector):
        self.vectorx = vector.x
        self.vectory = vector.y
        self.vectorz = vector.z

    def set_angle(self, angle):
        self.angle = angle / (2 * math.pi) * 360


class ParticleSystem:
    def __init__(self, program, source, viewing_angle):
        self.program = program
        self.current_time = 0
        self.past_time = 0
        self.source = source
        self.particles: List[Particle] = []
        self.viewing_angle = viewing_angle
        self.iteration = 0

    def update(self):
        self.current_time = glutGet(GLUT_ELAPSED_TIME)
        if self.current_time - self.past_time > 20:
            self.iteration += 1
            self.create_particles()
            self.refresh_particles()
            self.past_time = self.current_time
        self.program.my_display()

    def create_particles(self):
        for j in range(3):
            y = random.uniform(-1, 1)
            x = random.uniform(-1, 1)
            z = random.uniform(-1, 1)
            norm = (x ** 2 + y ** 2 + z ** 2) ** 0.5
            x /= norm
            y /= norm
            z /= norm
            self.particles.append(Particle(Vertex(x, y, z), 0.5, self.source))

    def refresh_particles(self):
        for particle in self.particles:
            vector, angle = self.particle_data(particle)
            particle.set_angle(angle)
            particle.set_vector(vector)
            particle.change_particle_position()
            particle.t -= 1
            particle.change_particle_color_and_size()
            self.kill_particle(particle)

    def particle_data(self, particle):
        s = Vertex(0, 0, 1)
        e = Vertex(particle.pos.x - self.viewing_angle.x, particle.pos.y - self.viewing_angle.y,
                   particle.pos.z - self.viewing_angle.z)
        vector = Vertex(s.y * e.z - e.y * s.z, e.x * s.z - s.x * e.z, s.x * e.y - s.y * e.x)
        s_norm = (s.x ** 2 + s.y ** 2 + s.z ** 2) ** 0.5
        e_norm = (e.x ** 2 + e.y ** 2 + e.z ** 2) ** 0.5
        se = s.x * e.x + s.y * e.y + s.z * e.z
        angle = acos(se / (s_norm * e_norm))
        return vector, angle

    def kill_particle(self, particle):
        if particle.t <= 0:
            self.particles.remove(particle)

    def draw_particles(self):
        for particle in self.particles:
            particle.draw_particle()


def load_texture(filename):
    surface = pg.image.load(filename)
    data = pg.image.tostring(surface, "RGB")
    texture = glGenTextures(1)
    glBindTexture(GL_TEXTURE_2D, texture)
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP)
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, surface.get_width(), surface.get_height(), GL_RGB, GL_UNSIGNED_BYTE, data)
    return texture


class Program:
    def __init__(self):
        self.viewing_angle = Vertex(0, 0, 50)
        self.source = Source(Vertex(0, 0, 0), (1, 0, 0), 1)
        self.particle_system = ParticleSystem(self, self.source, self.viewing_angle)

    def my_display(self):
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        glLoadIdentity()
        glTranslatef(self.viewing_angle.x, self.viewing_angle.y, -self.viewing_angle.z)
        self.particle_system.draw_particles()
        glutSwapBuffers()

    def my_reshape(self, w, h):
        global WIDTH, HEIGHT
        WIDTH = w
        HEIGHT = h
        glViewport(0, 0, WIDTH, HEIGHT)
        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        gluPerspective(45, WIDTH / HEIGHT, 0.1, 150)
        glMatrixMode(GL_MODELVIEW)
        glLoadIdentity()
        glLoadIdentity()
        glClearColor(0, 0, 0, 0)
        glClear(GL_COLOR_BUFFER_BIT)
        glPointSize(1)
        glColor3f(0, 0, 0)

    def main(self):
        glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB)
        glutInitWindowSize(WIDTH, HEIGHT)
        glutInitWindowPosition(10, 10)
        glutInit()
        self.window = glutCreateWindow("2. laboratorijska vjezba")
        glutReshapeFunc(self.my_reshape)
        glutDisplayFunc(self.my_display)
        glutIdleFunc(self.particle_system.update)
        self.texture = load_texture("particle.bmp")
        glBlendFunc(GL_SRC_ALPHA, GL_ONE)
        glEnable(GL_BLEND)
        glEnable(GL_TEXTURE_2D)
        glBindTexture(GL_TEXTURE_2D, self.texture)
        glutMainLoop()


if __name__ == '__main__':
    p = Program()
    p.main()
