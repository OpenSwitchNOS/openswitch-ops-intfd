# Design of interface daemon arbiter
The interface daemon arbiter is responsible for determining the forwarding state of an interface based on the forwarding state of the various sublayers operating at the interface layer.

The following flow chart describes the sequence of how the arbiter determines the final forwarding state of an interface.

```
+--------------------------+
| For every forwarding     +<---------------------------------+
| layer for this interface |                                  |
+-----------+--------------+                                  |
            |                                                 |
            |                                                 |
            |                                                 |
+-----------v---------------+       +----------------------+  |
| Is the previous forwarding| Yes   |Set the current layer |  |
| layer blocked             +----^-->as blocked and clear  |  |
+-----------+---------------+    |  |any owner set.        |  |
            | No                 |  +--------+-------------+  |
            |                    |           |                |
+-----------v---------------+ No |  +--------v-------------+  |
| Is the interface operator +----+  |Move to the next      |  |
| and H/W ready state UP?   |       |forwarding layer      +<-+----+
+-----------+---------------+       +---------^------------+       |
            | Yes                             |                    |
            |                                 |                    |
+-----------v---------------+       +---------+----------------+   |
| For every protocol        |       |Set the forwarding state  |   |
| operating at this layer   |       |of the layer as forwarding+<--------+
+-----------+---------------+       |and clear any owner set.  |   |     |
            |                       +--------------------------+   |     |
            |                                                      |     |
            |                                                      |     |
+-----------v---------------+                                      |     |
| Is the protocol's view of |  No                                  |     |
| the forwarding state of   +----+  +--------------------------+   |     |
| the interface blocked?    |    |  |Is the forwarding state   |   |     |
+-----------+---------------+    |  |of the interface blocked  | No|     |
            |                    +-->and the current owner     +------+  |
            | Yes                   |is this protocol          |   |  |  |
            |                       +-----------+--------------+   |  |  |
            |                                   |                  |  |  |
+-----------v---------------+                   | Yes              |  |  |
| Is the precedence of the  |                   |                  |  |  |
| current owner lower than  +---+               |                  |  |  |
| this protocol             |   |   +-----------v--------------+   |  |  |
+-----------+---------------+   |   |Set the forwarding state  |   |  |  |
            |                   |   |of the layer as forwarding|   |  |  |
            |                   |   |and clear the owner.      |   |  |  |
            | Yes               |   |                          |   |  |  |
            |                   |   +-----------+--------------+   |  |  |
            |                   |               |                  |  |  |
+-----------v---------------+   |               |                  |  |  |
| Set this protocol as the  |   |               |                  |  |  |
| new owner                 |   |   +-----------v--------------+   |  |  |
+-----------+---------------+   +--->Move to the next protocol <------+  |
            |                       +-----------+--------------+   |     |
            |                                   |                  |     |
            |                                   +------------------------+
            +------------------------------------------------------^



```
