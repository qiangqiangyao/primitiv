#include <config.h>

#include <vector>
#include <primitiv/function_impl.h>
#include <primitiv/graph.h>
#include <primitiv/node.h>
#include <primitiv/node_ops.h>
#include <primitiv/shape.h>
#include <primitiv/parameter.h>

namespace primitiv {

Node operator+(const Node &x) { return x.graph()->add_function(new functions::Positive(), {x}); }
Node operator-(const Node &x) { return x.graph()->add_function(new functions::Negative(), {x}); }
Node operator+(const Node &x, float k) { return x.graph()->add_function(new functions::AddConst(k), {x}); }
Node operator+(float k, const Node &x) { return x.graph()->add_function(new functions::AddConst(k), {x}); }
Node operator+(const Node &a, const Node &b) { return a.graph()->add_function(new functions::Add(), {a, b}); }
Node operator-(const Node &x, float k) { return x.graph()->add_function(new functions::SubtractConstR(k), {x}); }
Node operator-(float k, const Node &x) { return x.graph()->add_function(new functions::SubtractConstL(k), {x}); }
Node operator-(const Node &a, const Node &b) { return a.graph()->add_function(new functions::Subtract(), {a, b}); }
Node operator*(const Node &x, float k) { return x.graph()->add_function(new functions::MultiplyConst(k), {x}); }
Node operator*(float k, const Node &x) { return x.graph()->add_function(new functions::MultiplyConst(k), {x}); }
Node operator*(const Node &a, const Node &b) { return a.graph()->add_function(new functions::Multiply(), {a, b}); }
Node operator/(const Node &x, float k) { return x.graph()->add_function(new functions::DivideConstR(k), {x}); }
Node operator/(float k, const Node &x) { return x.graph()->add_function(new functions::DivideConstL(k), {x}); }
Node operator/(const Node &a, const Node &b) { return a.graph()->add_function(new functions::Divide(), {a, b}); }

namespace node_ops {

Node input(const Shape &shape, const std::vector<float> &data, Device *dev, Graph *g) {
  return g->add_function(new functions::Input(shape, data, dev), {});
}

Node input(Parameter *param, Graph *g) {
  return g->add_function(new functions::ParameterInput(param), {});
}

Node copy(const Node &x, Device *dev) {
  return x.graph()->add_function(new functions::Copy(dev), {x});
}

Node pick(const Node &x, unsigned dim, const std::vector<unsigned> &ids) {
  return x.graph()->add_function(new functions::Pick(dim, ids), {x});
}

Node slice(const Node &x, unsigned dim, unsigned lower, unsigned upper) {
  return x.graph()->add_function(new functions::Slice(dim, lower, upper), {x});
}

Node transpose(const Node &x) {
  return x.graph()->add_function(new functions::Transpose(), {x});
}

Node dot(const Node &a, const Node &b) {
  return a.graph()->add_function(new functions::Dot(), {a, b});
}

Node sqrt(const Node &x) {
  return x.graph()->add_function(new functions::Sqrt(), {x});
}

Node exp(const Node &x) {
  return x.graph()->add_function(new functions::Exp(), {x});
}

Node tanh(const Node &x) {
  return x.graph()->add_function(new functions::Tanh(), {x});
}

Node sigmoid(const Node &x) {
  return x.graph()->add_function(new functions::Sigmoid(), {x});
}

Node relu(const Node &x) {
  return x.graph()->add_function(new functions::ReLU(), {x});
}

Node sum(const Node &x, unsigned dim) {
  return x.graph()->add_function(new functions::Sum(dim), {x});
}

Node mean(const Node &x, unsigned dim) {
  return (1. / x.shape()[dim]) * sum(x, dim);
}

Node logsumexp(const Node &x, unsigned dim) {
  return x.graph()->add_function(new functions::LogSumExp(dim), {x});
}

Node log_softmax(const Node &x, unsigned dim) {
  return x - broadcast(logsumexp(x, dim), dim, x.shape()[dim]);
}

Node softmax(const Node &x, unsigned dim) {
  return exp(log_softmax(x, dim));
}

Node broadcast(const Node &x, unsigned dim, unsigned size) {
  return x.graph()->add_function(new functions::Broadcast(dim, size), {x});
}

Node softmax_cross_entropy(const Node &x, const Node &t, unsigned dim) {
  return x.graph()->add_function(new functions::SoftmaxCrossEntropy(dim), {x, t});
}

Node softmax_cross_entropy(const Node &x, unsigned dim, const std::vector<unsigned> &ids) {
  return pick(-log_softmax(x, dim), dim, ids);
}

Node dropout(const Node &x, float rate, bool enabled) {
  if (!enabled) return x;
  if (rate == 1.) return 0. * x;
  const float p = 1. - rate;
  return (1. / p) * x * random::bernoulli(x.shape(), p, x.device(), x.graph());
}

namespace batch {

Node sum(const Node &x) {
  return x.graph()->add_function(new functions::BatchSum(), {x});
}

Node mean(const Node &x) {
  return (1. / x.shape().batch_size()) * sum(x);
}

Node normalize(const Node &x) {
  if (!x.shape().has_batch()) return x;  // No meaning of normalization.
  const unsigned b = x.shape().batch_size();
  const float scale = b / (b - 1.);
  const Node m = mean(x);
  const Node v = scale * (mean(x * x) - m * m);
  return (x - m) / sqrt(v + 1e-8);
}

}  // namespace batch

namespace random {

Node bernoulli(const Shape &shape, float p, Device *dev, Graph *g) {
  return g->add_function(new functions::RandomBernoulli(shape, p, dev), {});
}

Node uniform(const Shape &shape, float lower, float upper, Device *dev, Graph *g) {
  return g->add_function(new functions::RandomUniform(shape, lower, upper, dev), {});
}

Node normal(const Shape &shape, float mean, float sd, Device *dev, Graph *g) {
  return g->add_function(new functions::RandomNormal(shape, mean, sd, dev), {});
}

Node log_normal(const Shape &shape, float mean, float sd, Device *dev, Graph *g) {
  return g->add_function(new functions::RandomLogNormal(shape, mean, sd, dev), {});
}

}  // namespace random

}  // namespace node_ops
}  // namespace primitiv
