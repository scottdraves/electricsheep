--
--  Merge function implementing a skew heap.
--  The result is written to r.right.
--
--  Skew heaps have an amortized complexity of O(log n)
--  and are simple to implement. The data structure
--  is a heap-ordered binary tree. The basic operation
--  is the merge operation which merges two heaps into one.
--
require "class"

module( ..., package.seeall )

local pq_mt = class.create( _M )

--	Return new instance.
function _M:new()
	return setmetatable( { m_Queue = {} }, pq_mt )
end

--
function _M:skew_merge( a, b, r )

	if not b then
		r.right = a
    else
		while a do
			if a.prio <= b.prio then
				r.right, r = a, a
				a.left, a = a.right, a.left
			else
				r.right, r = b, b
				b.left, a, b = b.right, b.left, a
			end
		end

		r.right = b
    end
end

--  Empty test
function _M:empty()
    return self.m_Queue.right == nil
end

--  Clear test
function _M:clear()
	while not self:empty() do
		self:remove()
	end
end

--  Insert into queue.	Merge the new node indo the heap.
function _M:insert( v )
	_M:skew_merge( self.m_Queue.right, v, self.m_Queue )
end

--	Take the root from the heap. Merge its two children to obtain the new root
function _M:remove()
    local r = self.m_Queue.right
    assert( r ~= nil, "remove called on empty queue" )
    self:skew_merge( r.left, r.right, self.m_Queue )
    r.left, r.right = nil, nil
    return r
end
